#include "concurrency/SLSPSCQueue.h"
#include "concurrency/implementation_details/SPSCQueueMeta.h"
#include <string.h>
#include <atomic>


using namespace Gicame;
using namespace Gicame::Concurrency;
using namespace Gicame::Concurrency::Impl;


SLSPSCQueue::SLSPSCQueue(void* buffer, const size_t capacity, const BufferWrappingStrategy bws) :
	SPSCQueue(buffer, capacity, bws) {}

SLSPSCQueue::~SLSPSCQueue() {}

void SLSPSCQueue::waitElemPresent(const size_t dataSize) {
	size_t s;
	do {
		s = size();
	} while (s < dataSize);
}

void SLSPSCQueue::waitFreeSpace(const size_t dataSize) {
	size_t s;
	do {
		s = size();
	} while (capacity - s - 1u < dataSize);
}
