#include "concurrency/InterprocessMutex.h"
#include "os/ObjectName.h"
#include "utils/Memory.h"
#include "utils/Numbers.h"
#include <chrono>
#ifdef WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <atomic>
#include <thread>
#include <new>
#endif


using namespace Gicame;
using namespace Gicame::Concurrency;
using AcquireResult = InterprocessMutex::AcquireResult;


#ifndef WINDOWS

namespace Gicame::Concurrency::Impl {

	struct RobustMutexData {
		// Any process may arrive first, so there is no master to initialize the mutex: whoever
		// moves state from NOT_INITIALIZED to INITIALIZING does it, the others wait for READY.
		static constexpr uint32_t NOT_INITIALIZED = 0u;  // A fresh mapping is zero-filled
		static constexpr uint32_t INITIALIZING = 1u;
		static constexpr uint32_t READY = 2u;

		pthread_mutex_t mutex;
		std::atomic<uint32_t> state;
	};

	// An atomic shared between processes only works if it is lock-free.
	static_assert(std::atomic<uint32_t>::is_always_lock_free);

};

#endif


#ifdef WINDOWS

static AcquireResult toAcquireResult(const DWORD waitResult) noexcept {
	switch (waitResult) {
	case WAIT_OBJECT_0:
		return AcquireResult::SUCCESS;
	case WAIT_ABANDONED:  // The owner thread ended without releasing it: we get it anyway
		return AcquireResult::OWNER_DIED;
	case WAIT_TIMEOUT:
		return AcquireResult::TIMEOUT;
	default:
		return AcquireResult::FAIL;
	}
}

#else

static AcquireResult toAcquireResult(pthread_mutex_t* mutex, const int error) noexcept {
	switch (error) {
	case 0:
		return AcquireResult::SUCCESS;
	case EOWNERDEAD:
		// The lock is ours, but unless it is marked consistent the mutex becomes unusable for
		// everybody once it is unlocked.
		if (pthread_mutex_consistent(mutex) != 0) {
			pthread_mutex_unlock(mutex);
			return AcquireResult::FAIL;
		}
		return AcquireResult::OWNER_DIED;
	case ETIMEDOUT:
		return AcquireResult::TIMEOUT;
	default:
		return AcquireResult::FAIL;
	}
}

#endif


InterprocessMutex::InterprocessMutex(const std::string& name) :
#ifdef WINDOWS
	handle(NULL),
#else
	shmem(std::string("im_shmem_") + name, sizeof(Impl::RobustMutexData)),
	sharedData(NULL),
#endif
	previousOwnerDead(false)
{
#ifdef WINDOWS

	if (!Os::isValidObjectName(name))
		throw RUNTIME_ERROR("invalid mutex name: " + name);

	handle = CreateMutexA(NULL, FALSE, (std::string("Local\\") + name).c_str());
	if (!handle)
		throw RUNTIME_ERROR("CreateMutexA(...) failed");

#else

	if (!shmem.open(true))
		throw RUNTIME_ERROR("Cannot open shared memory");
	// Until this instance turns out to be the initializer: leaving after a failure must not
	// remove a name that other processes may be using.
	shmem.setUnlinkOnDestruction(false);

	const auto [memPtr, newSize] = Utilities::align<Impl::RobustMutexData>(shmem.get(), shmem.getSize());
	if (!memPtr)
		throw RUNTIME_ERROR("Unaligned shared memory error");
	sharedData = std::launder(reinterpret_cast<Impl::RobustMutexData*>(memPtr));

	uint32_t expected = Impl::RobustMutexData::NOT_INITIALIZED;
	if (sharedData->state.compare_exchange_strong(expected, Impl::RobustMutexData::INITIALIZING)) {
		pthread_mutexattr_t attr;
		if (pthread_mutexattr_init(&attr) != 0 ||
			pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0 ||
			pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST) != 0 ||
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0 ||
			pthread_mutex_init(&(sharedData->mutex), &attr) != 0) {
			// Let the next instance try again instead of waiting for READY in vain.
			sharedData->state.store(Impl::RobustMutexData::NOT_INITIALIZED);
			throw RUNTIME_ERROR("pthread_mutex_init failed");
		}
		pthread_mutexattr_destroy(&attr);

		sharedData->state.store(Impl::RobustMutexData::READY, std::memory_order_release);
		shmem.setUnlinkOnDestruction(true);
	}
	else {
		// Initializing takes microseconds: a longer wait means the initializer died meanwhile.
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
		while (sharedData->state.load(std::memory_order_acquire) != Impl::RobustMutexData::READY) {
			if (std::chrono::steady_clock::now() > deadline)
				throw RUNTIME_ERROR("Mutex not initialized by the process that created it");
			std::this_thread::yield();
		}
	}

#endif
}

InterprocessMutex::~InterprocessMutex() {
#ifdef WINDOWS
	CloseHandle(handle);
#endif
	// On POSIX shmem unmaps the mutex and, for the initializer, removes its name.
}

AcquireResult InterprocessMutex::recordAcquisition(const AcquireResult result) noexcept {
	// Written only by the thread that now holds the mutex, so the holder reads its own result.
	if (result == AcquireResult::SUCCESS || result == AcquireResult::OWNER_DIED)
		previousOwnerDead = (result == AcquireResult::OWNER_DIED);
	return result;
}

AcquireResult InterprocessMutex::acquire() {
#ifdef WINDOWS
	return recordAcquisition(toAcquireResult(WaitForSingleObject(handle, INFINITE)));
#else
	return recordAcquisition(toAcquireResult(&(sharedData->mutex), pthread_mutex_lock(&(sharedData->mutex))));
#endif
}

AcquireResult InterprocessMutex::acquireWithTimeout(const std::chrono::milliseconds timeout) {
#ifdef WINDOWS

	const DWORD timeoutMs = Utilities::safeNumericCast<DWORD>(timeout.count());
	return recordAcquisition(toAcquireResult(WaitForSingleObject(handle, timeoutMs)));

#else

	// pthread_mutex_timedlock requires an absolute CLOCK_REALTIME time
	struct timespec absTime;
	if (clock_gettime(CLOCK_REALTIME, &absTime) != 0)
		return AcquireResult::FAIL;
	const long long totalNs = absTime.tv_nsec + (timeout.count() * 1000000LL);
	absTime.tv_sec += totalNs / 1000000000LL;
	absTime.tv_nsec = totalNs % 1000000000LL;

	return recordAcquisition(toAcquireResult(&(sharedData->mutex), pthread_mutex_timedlock(&(sharedData->mutex), &absTime)));

#endif
}

bool InterprocessMutex::release() {
#ifdef WINDOWS
	return ReleaseMutex(handle) != 0;
#else
	return pthread_mutex_unlock(&(sharedData->mutex)) == 0;
#endif
}

void InterprocessMutex::lock() {
	if (acquire() == AcquireResult::FAIL)
		throw RUNTIME_ERROR("Unable to lock the mutex");
}

void InterprocessMutex::unlock() {
	if (!release())
		throw RUNTIME_ERROR("Unable to unlock the mutex");
}

bool InterprocessMutex::try_lock() {
	// Both systems take the lock with a zero timeout whenever it is free right away.
	const AcquireResult result = acquireWithTimeout(std::chrono::milliseconds(0));
	if (result == AcquireResult::FAIL)
		throw RUNTIME_ERROR("Unable to lock the mutex");
	return result != AcquireResult::TIMEOUT;
}

bool InterprocessMutex::isPreviousOwnerDead() const noexcept {
	return previousOwnerDead;
}

void InterprocessMutex::setUnlinkOnDestruction(bool uod) {
#ifdef WINDOWS
	UNUSED(uod);
#else
	shmem.setUnlinkOnDestruction(uod);
#endif
}
