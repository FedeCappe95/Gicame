#include "concurrency/InterprocessQueue.h"
#include "concurrency/implementation_details/CircularBuffer.h"
#include "utils/Memory.h"
#include <atomic>
#include <string>
#include <new>
#include <cstddef>
#include <algorithm>
#ifdef WINDOWS
#include <Windows.h>
#endif


#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


using namespace Gicame;
using namespace Gicame::Concurrency;
using namespace Gicame::Concurrency::Impl;


void InterprocessQueue::waitElemPresent(const size_t dataSize) {
	size_t present = size();
	while (present < dataSize) {
		dataPresentEvent.wait();
		present = size();
	}
}

void InterprocessQueue::waitFreeSpace(const size_t dataSize) {
	size_t free = freeSpace();
	while (free < dataSize) {
		dataFreeEvent.wait();
		free = freeSpace();
	}
}

InterprocessQueue::InterprocessQueue(const std::string& name, const size_t capacity_, const ConcurrencyRole cr) :
	header(NULL),
	buffer(NULL),
	capacity(0),
	shmem(std::string("iq_shmem_") + name, capacity_ + sizeof(Gicame::Concurrency::Impl::CircularBufferDescriptor) + alignof(std::max_align_t)),
	dataPresentEvent(std::string("iq_dataPresentEvent_") + name, cr),
	dataFreeEvent(std::string("iq_dataFreeEvent_") + name, cr)
{
	constexpr ipc_size_t maxCapacity = ~ipc_size_t(0);
	if (capacity_ > maxCapacity)
		throw RUNTIME_ERROR("Capacity too big");

	const bool success = shmem.open(cr == ConcurrencyRole::MASTER);
	if (!success)
		throw RUNTIME_ERROR("Unable to open shared memory");

	const auto[memPtr, newSize] = Utilities::align<CircularBufferDescriptor>(shmem.get(), shmem.getSize());
	if (!memPtr)
		throw RUNTIME_ERROR("Insufficient capacity");

	header = new (memPtr) CircularBufferDescriptor;
	buffer = Utilities::advance<uint8_t>(memPtr, sizeof(CircularBufferDescriptor));
	capacity = newSize - sizeof(CircularBufferDescriptor);
	capacity = std::min(capacity, capacity_);  // not to go over capacity_
	if (capacity < 2u)
		throw RUNTIME_ERROR("Insufficient capacity");

	if (cr == ConcurrencyRole::MASTER) {
		header->head = 0;
		header->tail = 0;
	}
}

InterprocessQueue::~InterprocessQueue() {}

void InterprocessQueue::push(const void* data, size_t dataSize) {
	const uint8_t* ptr = static_cast<const uint8_t*>(data);

	while (dataSize) {
		const size_t chunkSize = likely(dataSize < (capacity - 1u)) ? dataSize : (capacity - 1u);

		waitFreeSpace(chunkSize);

		const ipc_size_t h = header->head.load();

		for (size_t i = 0; i < chunkSize; ++i)
			buffer[(h + i) % capacity] = ptr[i];

		header->head.store(static_cast<ipc_size_t>((h + chunkSize) % capacity));

		dataPresentEvent.signal();

		dataSize -= chunkSize;
		ptr = ptr + chunkSize;
	}
}

void InterprocessQueue::pop(void* outBuffer, size_t dataSize) {
	uint8_t* ptr = static_cast<uint8_t*>(outBuffer);

	while (dataSize) {
		const size_t chunkSize = likely(dataSize < (capacity - 1u)) ? dataSize : (capacity - 1u);

		waitElemPresent(chunkSize);

		const ipc_size_t t = header->tail.load();

		for (size_t i = 0; i < chunkSize; ++i)
			ptr[i] = buffer[(t + i) % capacity];

		header->tail.store(static_cast<ipc_size_t>((t + chunkSize) % capacity));

		dataFreeEvent.signal();

		dataSize -= chunkSize;
		ptr += chunkSize;
	}
}

size_t InterprocessQueue::size() {
	const ipc_size_t h = header->head.load();
	const ipc_size_t t = header->tail.load();
	if (h >= t)
		return static_cast<size_t>(h - t);
	else
		return static_cast<size_t>(capacity - (t - h));
}

size_t InterprocessQueue::freeSpace() {
	return capacity - size() - 1u;
}
