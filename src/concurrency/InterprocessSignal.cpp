#include "concurrency/InterprocessSignal.h"
#include "utils/Memory.h"
#include "configuration.h"
#include <utility>
#include <atomic>
#ifdef GICAME_USE_FUTEX
#include "futex/Futex.h"
#endif
#if defined(WINDOWS)
#include <Windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <new>
#endif


using namespace Gicame;
using namespace Gicame::Concurrency;
using namespace Gicame::Concurrency::Impl;


#if !defined(WINDOWS) && !defined(GICAME_USE_FUTEX)

namespace Gicame::Concurrency::Impl {

	struct PosixMutexCV {
		pthread_mutex_t mutex;
		pthread_cond_t cond;
		uint32_t signaled;
		// Set last by the master: a zero-filled pthread object is not a valid process-shared one.
		std::atomic<uint32_t> initialized;
	};

	// An atomic shared between processes only works if it is lock-free.
	static_assert(std::atomic<uint32_t>::is_always_lock_free);

};

#endif


Gicame::Concurrency::InterprocessSignal::InterprocessSignal(const std::string& name, const ConcurrencyRole cr) :
#if defined(WINDOWS)
	eventHandle(NULL)
#elif defined(GICAME_USE_FUTEX)
	shmem(std::string("is_shmem_") + name, sizeof(*futexp)),
	futexp(NULL)
#else
	shmem(std::string("is_shmem_") + name, sizeof(PosixMutexCV)),
	sharedData(NULL)
#endif
{
#if defined(WINDOWS)

	UNUSED(cr);
	const std::string eventName = std::string("is_") + name;
	eventHandle = CreateEventA(NULL, FALSE, FALSE, eventName.c_str());
	if (!eventHandle)
		throw RUNTIME_ERROR("CreateEvent failed, check signal name");

#elif defined(GICAME_USE_FUTEX)

	const bool success = shmem.open(cr == ConcurrencyRole::MASTER);
	if (!success)
		throw RUNTIME_ERROR("Cannot create shared memory");
	shmem.setUnlinkOnDestruction(cr == ConcurrencyRole::MASTER);

	const auto[memPtr, newSize] = Utilities::align<std::atomic<uint32_t>>(shmem.get(), shmem.getSize());
	if (!memPtr)
		throw RUNTIME_ERROR("Unaligned shared memory error");

	// Only the master resets the word: a slave doing it would drop a pending signal. A fresh
	// mapping is zero-filled, which already means "not signaled".
	if (cr == ConcurrencyRole::MASTER) {
		futexp = new (memPtr) std::atomic<uint32_t>;
		futexp->store(0u);
	}
	else {
		futexp = std::launder(reinterpret_cast<std::atomic<uint32_t>*>(memPtr));
	}

#else

	const bool success = shmem.open(cr == ConcurrencyRole::MASTER);
	if (!success)
		throw RUNTIME_ERROR("Cannot create shared memory");
	shmem.setUnlinkOnDestruction(cr == ConcurrencyRole::MASTER);

	const auto[memPtr, newSize] = Utilities::align<PosixMutexCV>(shmem.get(), shmem.getSize());
	if (!memPtr)
		throw RUNTIME_ERROR("Unaligned shared memory error");

	if (cr == ConcurrencyRole::MASTER) {
		sharedData = new (memPtr) PosixMutexCV;
		sharedData->signaled = 0u;
		// A mapping left behind by a crashed run may still read as initialized.
		sharedData->initialized.store(0u, std::memory_order_relaxed);

		pthread_mutexattr_t mutexAttr;
		if (pthread_mutexattr_init(&mutexAttr) != 0 ||
			pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED) != 0 ||
			pthread_mutex_init(&(sharedData->mutex), &mutexAttr) != 0)
			throw RUNTIME_ERROR("pthread_mutex_init failed");
		pthread_mutexattr_destroy(&mutexAttr);

		pthread_condattr_t condAttr;
		if (pthread_condattr_init(&condAttr) != 0 ||
			pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED) != 0 ||
			pthread_cond_init(&(sharedData->cond), &condAttr) != 0)
			throw RUNTIME_ERROR("pthread_cond_init failed");
		pthread_condattr_destroy(&condAttr);

		sharedData->initialized.store(1u, std::memory_order_release);
	}
	else {
		// Re-initializing objects the master may already be using is undefined behaviour.
		sharedData = std::launder(reinterpret_cast<PosixMutexCV*>(memPtr));
		if (sharedData->initialized.load(std::memory_order_acquire) != 1u)
			throw RUNTIME_ERROR("Signal not initialized by the master yet");
	}

#endif
}

Gicame::Concurrency::InterprocessSignal::InterprocessSignal(InterprocessSignal&& other) :
#if defined(WINDOWS)
	eventHandle(other.eventHandle)
#elif defined(GICAME_USE_FUTEX)
	shmem(std::move(other.shmem)),
	futexp(other.futexp)
#else
	shmem(std::move(other.shmem)),
	sharedData(other.sharedData)
#endif
{
	// Leave the source inert: its destructor must not release what this object has just taken
	// over. On the non-Windows branches the mapping itself is owned by shmem, whose move
	// constructor already cleared the source; the pointers below merely point into it.
#if defined(WINDOWS)
	other.eventHandle = NULL;
#elif defined(GICAME_USE_FUTEX)
	other.futexp = NULL;
#else
	other.sharedData = NULL;
#endif
}

Gicame::Concurrency::InterprocessSignal::~InterprocessSignal() {
#if defined(WINDOWS)
	// NULL after a move: CloseHandle(NULL) is an invalid-handle call and raises under a debugger
	// with invalid-handle checks enabled.
	if (eventHandle)
		CloseHandle(eventHandle);
#endif
}

WaitResult Gicame::Concurrency::InterprocessSignal::wait()
{
#if defined(WINDOWS)

	const DWORD waitResult = WaitForSingleObject(eventHandle, INFINITE);
	if (waitResult == WAIT_TIMEOUT) return WaitResult::TIMEOUT_ELAPSED;
	if (waitResult == WAIT_FAILED) return WaitResult::FAILED;

#elif defined(GICAME_USE_FUTEX)

	fwait(futexp);

#else

	int errorCode = pthread_mutex_lock(&(sharedData->mutex));
	if (errorCode != 0)
		throw RUNTIME_ERROR("pthread_mutex_lock failed");

	// Looping on the flag absorbs spurious wakeups of pthread_cond_wait.
	int condWaitErrorCode = 0;
	while (!sharedData->signaled && condWaitErrorCode == 0)
		condWaitErrorCode = pthread_cond_wait(&(sharedData->cond), &(sharedData->mutex));
	if (condWaitErrorCode == 0)
		sharedData->signaled = 0u;

	errorCode = pthread_mutex_unlock(&(sharedData->mutex));
	if (errorCode != 0)
		throw RUNTIME_ERROR("pthread_mutex_unlock failed");

	if (condWaitErrorCode != 0) return WaitResult::FAILED;

#endif
	return WaitResult::OK;
}

void Gicame::Concurrency::InterprocessSignal::signal()
{
#if defined(WINDOWS)

	SetEvent(eventHandle);

#elif defined(GICAME_USE_FUTEX)

	fpost(futexp);

#else

	int errorCode = pthread_mutex_lock(&(sharedData->mutex));
	if (errorCode != 0)
		throw RUNTIME_ERROR("pthread_mutex_lock failed");

	sharedData->signaled = 1u;
	const int condSigErrorCode = pthread_cond_signal(&(sharedData->cond));

	errorCode = pthread_mutex_unlock(&(sharedData->mutex));
	if (errorCode != 0)
		throw RUNTIME_ERROR("pthread_mutex_unlock failed");

	if (condSigErrorCode != 0)
		throw RUNTIME_ERROR("pthread_cond_signal failed");

#endif
}
