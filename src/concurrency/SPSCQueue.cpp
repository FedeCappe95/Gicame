#include "concurrency/SPSCQueue.h"
#include "concurrency/implementation_details/CircularBuffer.h"
#include "utils/Memory.h"


using namespace Gicame;
using namespace Gicame::Concurrency;
using namespace Gicame::Concurrency::Impl;


void SPSCQueue::waitElemPresent(const size_t dataSize) {
	size_t present = size();
	while (present < dataSize) {
		dataPresentEvent.wait();
		present = size();
	}
}

bool SPSCQueue::waitElemPresent(const size_t dataSize, uint32_t timeoutMs) {
	if (size() >= dataSize)
		return true;

	// The signal may be already signaled
	dataPresentEvent.wait(0u);
	if (size() >= dataSize)
		return true;

	// Now wait
	dataPresentEvent.wait(timeoutMs);
	return size() >= dataSize;
}

void SPSCQueue::waitFreeSpace(const size_t dataSize) {
	size_t free = freeSpace();
	while (free < dataSize) {
		dataFreeEvent.wait();
		free = freeSpace();
	}
}

SPSCQueue::SPSCQueue(const size_t capacity_) :
	capacity(capacity_),
	head(0u),
	tail(0u)
{}

SPSCQueue::~SPSCQueue() {}

void SPSCQueue::push(const void* data, size_t dataSize) {
	const uint8_t* ptr = static_cast<const uint8_t*>(data);

	while (dataSize) {
		const size_t chunkSize = likely(dataSize < (capacity - 1u)) ? dataSize : (capacity - 1u);

		waitFreeSpace(chunkSize);

		const ipc_size_t h = head.load();

		for (size_t i = 0; i < chunkSize; ++i)
			buffer[(h + i) % capacity] = ptr[i];

		head.store(static_cast<ipc_size_t>((h + chunkSize) % capacity));

		dataPresentEvent.signal();

		dataSize -= chunkSize;
		ptr = ptr + chunkSize;
	}
}

void SPSCQueue::pop(void* outBuffer, size_t dataSize) {
	uint8_t* ptr = static_cast<uint8_t*>(outBuffer);

	while (dataSize) {
		const size_t chunkSize = likely(dataSize < (capacity - 1u)) ? dataSize : (capacity - 1u);

		waitElemPresent(chunkSize);

		const ipc_size_t t = tail.load();

		for (size_t i = 0; i < chunkSize; ++i)
			ptr[i] = buffer[(t + i) % capacity];

		tail.store(static_cast<ipc_size_t>((t + chunkSize) % capacity));

		dataFreeEvent.signal();

		dataSize -= chunkSize;
		ptr += chunkSize;
	}
}

bool SPSCQueue::pop(void* outBuffer, size_t dataSize, uint32_t timeoutMs) {
	if (dataSize > capacity - 1u)
		return false;

	if (!waitElemPresent(dataSize, timeoutMs))
		return false;

	const ipc_size_t t = tail.load();

	uint8_t* ptr = static_cast<uint8_t*>(outBuffer);
	for (size_t i = 0; i < dataSize; ++i)
		ptr[i] = buffer[(t + i) % capacity];

	tail.store(static_cast<ipc_size_t>((t + dataSize) % capacity));

	dataFreeEvent.signal();

	return true;
}

size_t SPSCQueue::size() const noexcept {
	const ipc_size_t h = head.load();
	const ipc_size_t t = tail.load();
	if (h >= t)
		return static_cast<size_t>(h - t);
	else
		return static_cast<size_t>(capacity - (t - h));
}

size_t SPSCQueue::freeSpace() const noexcept {
	return capacity - size() - 1u;
}
