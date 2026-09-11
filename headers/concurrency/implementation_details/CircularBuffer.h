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

	
	// Following code is unused, but its adoption may became reality
#if 0
	template<typename WFS, typename NEP>
	static inline void pushHeavy(const uint8_t* data, size_t dataSize, CircularBufferDescriptor* cbd, uint8_t* buffer, const size_t capacity, WFS waitFreeSpace, NEP notifyElemPresent) {
		while (dataSize) {
			const size_t chunkSize = likely(dataSize < (capacity - 1u)) ? dataSize : (capacity - 1u);

			waitFreeSpace(chunkSize);

			const size_t h = cbd->head.load();

			size_t done = 0;
			uint8_t* out = buffer;
			while (done < chunkSize && (size_t)(out) % sizeof(size_t) != 0) {
				*out = *(uint8_t*)(data);
				++done;
				++data;
				out = &buffer[(h + done) & capacity];
			}
			while (done < chunkSize && (chunkSize - done) > sizeof(size_t)) {
				*(size_t*)(out) = *(size_t*)(data);
				done += sizeof(size_t);
				data += sizeof(size_t);
				out = &buffer[(h + done) & capacity];
			}
			while (done < chunkSize) {
				*out = *(uint8_t*)(data);
				++done;
				++data;
				out = &buffer[(h + done) & capacity];
			}

			cbd->head.store((h + chunkSize) % capacity);

			notifyElemPresent();

			dataSize -= chunkSize;
		}
	}
#endif

};

#endif
