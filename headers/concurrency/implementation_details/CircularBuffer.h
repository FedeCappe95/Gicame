#ifndef __GICAME__CONCURRENCY__IMPLEMENTATION_DETAILS__CIRCULAR_BUFFER_H__
#define __GICAME__CONCURRENCY__IMPLEMENTATION_DETAILS__CIRCULAR_BUFFER_H__


#include "../../common.h"
#include "../../configuration.h"
#include <atomic>
#include <cstddef>


namespace Gicame::Concurrency::Impl {

	// Hardcoded on purpose: std::hardware_destructive_interference_size is C++17 but landed late
	// in libstdc++ and libc++, and it is ABI-fragile. 64 is right on every arch this runs on.
	static constexpr std::size_t CACHE_LINE_SIZE = 64u;

	// head is written only by the producer, tail only by the consumer, and the two sides usually
	// live in different processes on different cores. Keeping them on separate cache lines avoids
	// the line ping-pong that would otherwise happen on every single push/pop. The trailing
	// padding also pushes the ring buffer, which is laid out right after this descriptor, onto a
	// cache line of its own.
	// Single-producer/single-consumer is a precondition of the whole design: the index is read,
	// the buffer written and the index stored without atomicity across the three steps.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4324)  // padding added due to alignas: intentional here
#endif
	struct CircularBufferDescriptor {
		alignas(CACHE_LINE_SIZE) std::atomic<ipc_size_t> head;
		alignas(CACHE_LINE_SIZE) std::atomic<ipc_size_t> tail;
	};
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

	static inline size_t circularBufferSize(size_t cap, ipc_size_t h, ipc_size_t t) noexcept {
		return h >= t ? static_cast<size_t>(h - t) : (cap - static_cast<size_t>(t - h));
	}

	static inline size_t circularBufferFreeSpace(size_t cap, ipc_size_t h, ipc_size_t t) noexcept {
		const size_t size = h >= t ? static_cast<size_t>(h - t) : (cap - static_cast<size_t>(t - h));
		return cap - size - 1u;
	}

};

#endif
