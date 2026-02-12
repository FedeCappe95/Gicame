#include "concurrency/InterprocessSignal.h"
#include "utils/Memory.h"
#include "configuration.h"
#ifdef GICAME_USE_FUTEX
#include <atomic>
#include "futex/Futex.h"
#endif
#if defined(WINDOWS)
#include <Windows.h>
#else
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
	};

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

	futexp = new (memPtr) std::atomic<uint32_t>;
	*futexp = 0u;

#else

	const bool success = shmem.open(cr == ConcurrencyRole::MASTER);
	if (!success)
		throw RUNTIME_ERROR("Cannot create shared memory");
	shmem.setUnlinkOnDestruction(cr == ConcurrencyRole::MASTER);

	const auto[memPtr, newSize] = Utilities::align<PosixMutexCV>(shmem.get(), shmem.getSize());
	if (!memPtr)
		throw RUNTIME_ERROR("Unaligned shared memory error");

	sharedData = new (memPtr) PosixMutexCV;

	// mutex initialization
	pthread_mutexattr_t mutexAttr;
	pthread_mutexattr_init(&mutexAttr);
	pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&(sharedData->mutex), &mutexAttr);

	// cond initialization
	pthread_condattr_t condAttr;
	pthread_condattr_init(&condAttr);
	pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
	pthread_cond_init(&(sharedData->cond), &condAttr);

#endif
}

Gicame::Concurrency::InterprocessSignal::~InterprocessSignal() {
#if defined(WINDOWS)
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

	const int condWaitErrorCode = pthread_cond_wait(&(sharedData->cond), &(sharedData->mutex));

	errorCode = pthread_mutex_unlock(&(sharedData->mutex));
	if (errorCode != 0)
		throw RUNTIME_ERROR("pthread_mutex_unlock failed");

	if (condWaitErrorCode == ETIMEDOUT) return WaitResult::TIMEOUT_ELAPSED;
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

	const int condSigErrorCode = pthread_cond_signal(&(sharedData->cond));

	errorCode = pthread_mutex_unlock(&(sharedData->mutex));
	if (errorCode != 0)
		throw RUNTIME_ERROR("pthread_mutex_unlock failed");

	if (condSigErrorCode != 0)
		throw RUNTIME_ERROR("pthread_cond_signal failed");

#endif
}
