#include "concurrency/SPSCQueue.h"
#include "concurrency/implementation_details/SPSCQueueMeta.h"
#include <string.h>
#include <atomic>


using namespace Gicame;
using namespace Gicame::Concurrency;
using namespace Gicame::Concurrency::Impl;


SPSCQueue::SPSCQueue(void* buffer, const size_t capacity, const ConcurrencyRole cr) :
	meta((SPSCQueueMeta*)buffer),
	buffer((uint8_t*)buffer + sizeof(SPSCQueueMeta)),
	capacity(capacity - sizeof(SPSCQueueMeta))
{
	if (this->capacity > capacity || this->capacity < 2u)
		throw RUNTIME_ERROR("Insufficient capacity");
	if (cr == ConcurrencyRole::MASTER) {
		meta->head = 0;
		meta->tail = 0;
	}
}

SPSCQueue::~SPSCQueue() {}

void SPSCQueue::push(const void* data, const size_t dataSize) {
	waitFreeSpace(dataSize);
	const size_t h = meta->head.load();
	for (size_t i = 0; i < dataSize; ++i)
		buffer[(h + i) % capacity] = ((uint8_t*)data)[i];
	meta->head.store((h + dataSize) % capacity);
}

void SPSCQueue::pop(void* outBuffer, const size_t dataSize) {
	waitElemPresent(dataSize);
	const size_t t = meta->tail.load();
	for (size_t i = 0; i < dataSize; ++i)
		((uint8_t*)outBuffer)[i] = buffer[(t + i) % capacity];
	meta->tail.store((t + dataSize) % capacity);
}

size_t SPSCQueue::size() {
	const size_t h = meta->head.load();
	const size_t t = meta->tail.load();
	if (h >= t)
		return h - t;
	else
		return capacity - (t - h);
}