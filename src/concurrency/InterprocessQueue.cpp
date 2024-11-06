#include "concurrency/InterprocessQueue.h"
#include "concurrency/implementation_details/SPSCQueueMeta.h"
#include <atomic>
#include <string>
#ifdef WINDOWS
#include <Windows.h>
#endif


using namespace Gicame;
using namespace Gicame::Concurrency;


Gicame::Concurrency::Impl::SPSCQueueMeta* InterprocessQueue::getHeader() {
	return shmem.getAs<Gicame::Concurrency::Impl::SPSCQueueMeta>();
}

uint8_t* InterprocessQueue::getBuffer() {
	return shmem.getAs<uint8_t>() + sizeof(Gicame::Concurrency::Impl::SPSCQueueMeta);
}

void InterprocessQueue::waitElemPresent(const size_t dataSize) {
	size_t present = size();
	while (present < dataSize) {
#ifdef WINDOWS
		WaitForSingleObject(dataPresentEvent, INFINITE);
#endif
		present = size();
	}
}

void InterprocessQueue::waitFreeSpace(const size_t dataSize) {
	size_t free = freeSpace();
	while (free < dataSize) {
#ifdef WINDOWS
		WaitForSingleObject(dataFreeEvent, INFINITE);
#endif
		free = freeSpace();
	}
}

void InterprocessQueue::notifyElemPresent() {
#ifdef WINDOWS
	SetEvent(dataPresentEvent);
#endif
}

void InterprocessQueue::notifyFreeSpace() {
#ifdef WINDOWS
	SetEvent(dataFreeEvent);
#endif
}

InterprocessQueue::InterprocessQueue(const std::string& name, const size_t capacity, const ConcurrencyRole cr) :
	capacity(capacity),
	shmem(std::string("iq_shmem_") + name, capacity + sizeof(Gicame::Concurrency::Impl::SPSCQueueMeta)),
	sem(std::string("iq_sem_") + name, 1)
{
	if (capacity < 1u)
		RUNTIME_ERROR("Invalid capacity");

	if (cr == ConcurrencyRole::MASTER) {
		shmem.open(true);
		getHeader()->head = 0;
		getHeader()->tail = 0;
	}
	else {
		shmem.open(false);
	}

#ifdef WINDOWS
	const std::string dataPresentEventName = std::string("iq_dataPresentEvent_") + name;
	dataPresentEvent = CreateEventA(NULL, FALSE, FALSE, dataPresentEventName.c_str());
	const std::string dataFreeEventName = std::string("iq_dataFreeEvent_") + name;
	dataFreeEvent = CreateEventA(NULL, FALSE, FALSE, dataFreeEventName.c_str());
	if (dataPresentEvent == INVALID_HANDLE_VALUE || dataFreeEvent == INVALID_HANDLE_VALUE)
		RUNTIME_ERROR("CreateEventA(...) failed");

	mutex = CreateMutexA(NULL, cr == ConcurrencyRole::MASTER, "testmutex");
	ReleaseMutex(mutex);
#endif
}

InterprocessQueue::~InterprocessQueue() {}

void InterprocessQueue::push(const void* data, const size_t dataSize) {
	waitFreeSpace(dataSize);

	auto* header = getHeader();
	uint8_t* buffer = getBuffer();

	const size_t h = header->head.load();

	for (size_t i = 0; i < dataSize; ++i)
		buffer[(h + i) % capacity] = ((uint8_t*)data)[i];

	header->head.store((h + dataSize) % capacity);

	notifyElemPresent();
}

void InterprocessQueue::pop(void* outBuffer, const size_t dataSize) {
	waitElemPresent(dataSize);

	auto* header = getHeader();
	const uint8_t* buffer = getBuffer();

	const size_t t = header->tail.load();

	for (size_t i = 0; i < dataSize; ++i)
		((uint8_t*)outBuffer)[i] = buffer[(t + i) % capacity];

	header->tail.store((t + dataSize) % capacity);

	notifyFreeSpace();
}

size_t InterprocessQueue::size() {
	const auto* header = getHeader();
	const size_t h = header->head.load();
	const size_t t = header->tail.load();
	if (h >= t)
		return h - t;
	else
		return capacity - (t - h);
}

size_t InterprocessQueue::freeSpace() {
	return capacity - size() - 1u;
}
