#include "concurrency/SPSCQueue.h"
#include "concurrency/implementation_details/CircularBuffer.h"
#include "utils/Memory.h"
#include <string.h>
#include <atomic>
#include <memory>


using namespace Gicame;
using namespace Gicame::Concurrency;
using namespace Gicame::Concurrency::Impl;


SPSCQueue::SPSCQueue(void* buffer_, const size_t capacity_, const ConcurrencyRole cr) :
	header(NULL),
	buffer(NULL),
	capacity(0)
{
	constexpr ipc_size_t maxCapacity = ~ipc_size_t(0);
	if (capacity > maxCapacity)
		throw RUNTIME_ERROR("Capacity too big");

	const auto [memPtr, newSize] = Utilities::align<CircularBufferDescriptor>(buffer_, capacity_);
	if (!memPtr)
		throw RUNTIME_ERROR("Insufficient capacity");

	header = new (memPtr) CircularBufferDescriptor;
	buffer = Utilities::advance<uint8_t>(memPtr, sizeof(CircularBufferDescriptor));
	capacity = newSize - sizeof(CircularBufferDescriptor);
	if (capacity < 2u)
		throw RUNTIME_ERROR("Insufficient capacity");

	if (cr == ConcurrencyRole::MASTER) {
		header->head = 0;
		header->tail = 0;
	}
}

SPSCQueue::~SPSCQueue() {}

void SPSCQueue::push(const void* data, size_t dataSize) {
	const uint8_t* ptr = static_cast<const uint8_t*>(data);

	while (dataSize) {
		const size_t chunkSize = likely(dataSize < (capacity - 1u)) ? dataSize : (capacity - 1u);

		waitFreeSpace(chunkSize);

		const ipc_size_t h = header->head.load();

		for (size_t i = 0; i < chunkSize; ++i)
			buffer[(h + i) % capacity] = ptr[i];

		header->head.store(static_cast<ipc_size_t>((h + chunkSize) % capacity));

		dataSize -= chunkSize;
		ptr = ptr + chunkSize;
	}
}

void SPSCQueue::pop(void* outBuffer, size_t dataSize) {
	uint8_t* ptr = static_cast<uint8_t*>(outBuffer);

	while (dataSize) {
		const size_t chunkSize = likely(dataSize < (capacity - 1u)) ? dataSize : (capacity - 1u);

		waitElemPresent(chunkSize);

		const ipc_size_t t = header->tail.load();

		for (size_t i = 0; i < chunkSize; ++i)
			ptr[i] = buffer[(t + i) % capacity];

		header->tail.store(static_cast<ipc_size_t>((t + chunkSize) % capacity));

		dataSize -= chunkSize;
		ptr += chunkSize;
	}
}

size_t SPSCQueue::size() {
	const ipc_size_t h = header->head.load();
	const ipc_size_t t = header->tail.load();
	if (h >= t)
		return static_cast<size_t>(h - t);
	else
		return static_cast<size_t>(capacity - (t - h));
}