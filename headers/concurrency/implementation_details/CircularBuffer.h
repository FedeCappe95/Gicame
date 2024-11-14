#ifndef __GICAME__CONCURRENCY__IMPLEMENTATION_DETAILS__CIRCULAR_BUFFER_H__
#define __GICAME__CONCURRENCY__IMPLEMENTATION_DETAILS__CIRCULAR_BUFFER_H__


#include "../../common.h"
#include <atomic>


namespace Gicame::Concurrency::Impl {

	struct CircularBufferDescriptor {
		std::atomic<size_t> head;
		std::atomic<size_t> tail;
	};

	template<typename WFS, typename NEP>
	static inline void push(const uint8_t* data, size_t dataSize, CircularBufferDescriptor* cbd, uint8_t* buffer, const size_t capacity, WFS waitFreeSpace, NEP notifyElemPresent) {
		while (dataSize) {
			const size_t chunkSize = likely(dataSize < (capacity - 1u)) ? dataSize : (capacity - 1u);

			waitFreeSpace(chunkSize);

			const size_t h = cbd->head.load();
			for (size_t i = 0; i < chunkSize; ++i)
				buffer[(h + i) % capacity] = data[i];
			cbd->head.store((h + chunkSize) % capacity);

			notifyElemPresent();

			dataSize -= chunkSize;
			data += chunkSize;
		}
	}

	template<typename WEP, typename NFS>
	static inline void pop(uint8_t* outBuffer, size_t dataSize, CircularBufferDescriptor* cbd, uint8_t* buffer, const size_t capacity, WEP waitElemPresent, NFS notifyFreeSpace) {
		while (dataSize) {
			const size_t chunkSize = likely(dataSize < (capacity - 1u)) ? dataSize : (capacity - 1u);

			waitElemPresent(chunkSize);

			const size_t t = cbd->tail.load();
			for (size_t i = 0; i < chunkSize; ++i)
				outBuffer[i] = buffer[(t + i) % capacity];
			cbd->tail.store((t + chunkSize) % capacity);

			notifyFreeSpace();

			dataSize -= chunkSize;
			outBuffer += chunkSize;
		}
	}

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

};

#endif
