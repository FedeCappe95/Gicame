#include "concurrency/InterprocessSignal.h"
#ifdef WINDOWS
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#endif


using namespace Gicame;
using namespace Gicame::Concurrency;


#ifndef WINDOWS

struct PosixMutexCV {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

#endif


Gicame::Concurrency::InterprocessSignal::InterprocessSignal(const std::string& name, const ConcurrencyRole cr) :
#ifdef WINDOWS
	eventHandle(NULL)
#else
	shmem(std::string("is_shmem_") + name, sizeof(PosixMutexCV))
#endif
{
#ifdef WINDOWS
	UNUSED(cr);
	const std::string eventName = std::string("is_") + name;
	eventHandle = CreateEventA(NULL, FALSE, FALSE, eventName.c_str());
	if (!eventHandle)
		throw RUNTIME_ERROR("CreateEvent failed, check signal name");
#else
	const bool success = shmem.open(cr == ConcurrencyRole::MASTER);
	if (!success)
		throw RUNTIME_ERROR("Cannot create shared memory");
	shmem.setUnlinkOnDestruction(cr == ConcurrencyRole::MASTER);

	PosixMutexCV* sharedData = shmem.getAs<PosixMutexCV>();

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
#ifdef WINDOWS
	CloseHandle(eventHandle);
#endif
}

GICAME_API WaitResult Gicame::Concurrency::InterprocessSignal::wait()
{
#ifdef WINDOWS
	const DWORD waitResult = WaitForSingleObject(eventHandle, INFINITE);
	if (waitResult == WAIT_TIMEOUT) return WaitResult::TIMEOUT_ELAPSED;
	if (waitResult == WAIT_FAILED) return WaitResult::FAILED;
#else
	PosixMutexCV* sharedData = shmem.getAs<PosixMutexCV>();

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

GICAME_API void Gicame::Concurrency::InterprocessSignal::signal()
{
#ifdef WINDOWS
	SetEvent(eventHandle);
#else
	PosixMutexCV* sharedData = shmem.getAs<PosixMutexCV>();

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