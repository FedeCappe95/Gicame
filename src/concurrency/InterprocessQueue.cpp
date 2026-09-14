#include "concurrency/InterprocessQueue.h"
#include "concurrency/implementation_details/CircularBuffer.h"
#include "utils/Memory.h"
#include <atomic>
#include <string>
#include <new>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <tuple>


using namespace Gicame;
using namespace Gicame::Concurrency;
using namespace Gicame::Concurrency::Impl;


/*
 * ===== Help functions =====
 */

static constexpr size_t enlargeCapacity(size_t capa) noexcept {
	return capa +
		sizeof(Gicame::Concurrency::Impl::CircularBufferDescriptor) +
		alignof(Gicame::Concurrency::Impl::CircularBufferDescriptor);
}

static auto initShMem(SharedMemory& shmem, size_t capacity_, ConcurrencyRole cr) {
	constexpr ipc_size_t maxCapacity = ~ipc_size_t(0);
	if (capacity_ > maxCapacity)
		throw RUNTIME_ERROR("Capacity too big");

	const bool success = shmem.open(cr == ConcurrencyRole::MASTER);
	if (!success)
		throw RUNTIME_ERROR("Unable to open shared memory");

	const auto [memPtr, newSize] = Utilities::align<CircularBufferDescriptor>(shmem.get(), shmem.getSize());
	if (!memPtr)
		throw RUNTIME_ERROR("Insufficient capacity");

	CircularBufferDescriptor* newHeader = (cr == ConcurrencyRole::MASTER)
		? new (memPtr) CircularBufferDescriptor  // From C++20 placement new value initialise std::atomic
		: std::launder(reinterpret_cast<CircularBufferDescriptor*>(memPtr));
	uint8_t* newBuffer = Utilities::advance<uint8_t>(memPtr, sizeof(CircularBufferDescriptor));
	size_t newCapacity = std::min(newSize - sizeof(CircularBufferDescriptor), capacity_);  // not to go over capacity_

	return std::make_tuple(newHeader, newBuffer, newCapacity);
}


/*
 * ===== InterprocessQueue implementation =====
 */

InterprocessQueue::InterprocessQueue(const std::string& name, const size_t capacity_, const ConcurrencyRole cr) :
	ByteRing<Impl::IntPrQueueLockPolicy>(Impl::IntPrQueueLockPolicy(name, cr)),
	shmem(std::string("iq_shmem_") + name, enlargeCapacity(capacity_))
{
	auto [newHeader, newBuffer, newCapacity] = initShMem(shmem, capacity_, cr);
	initRing(newHeader, newBuffer, newCapacity, cr);
}

InterprocessQueue::~InterprocessQueue() {}


/*
 * ===== SLInterprocessQueue implementation =====
 */

SLInterprocessQueue::SLInterprocessQueue(const std::string& name, const size_t capacity_, const ConcurrencyRole cr) :
	ByteRing<Impl::SLIntPrQueueLockPolicy>(Impl::SLIntPrQueueLockPolicy{}),
	shmem(std::string("sliq_shmem_") + name, enlargeCapacity(capacity_))
{
	auto [newHeader, newBuffer, newCapacity] = initShMem(shmem, capacity_, cr);
	initRing(newHeader, newBuffer, newCapacity, cr);
}

SLInterprocessQueue::~SLInterprocessQueue() {}
