#ifndef __GICAME__CONCURRENCY__SPSPQUEUE_H__
#define __GICAME__CONCURRENCY__SPSPQUEUE_H__


#include "../common.h"
#include "../utils/NotCopyable.h"
#include "../interfaces/IDataExchanger.h"
#include "./Signal.h"
#include <stdint.h>
#include <vector>


// Forward declarations
namespace Gicame::Concurrency::Impl {
	struct CircularBufferDescriptor;
}


namespace Gicame::Concurrency {

	/**
	 * @brief Single Producer Single Consumer Queue implemented via circular buffer
	 */
	class SPSCQueue : public IDataExchanger {

		NOT_COPYABLE(SPSCQueue)

	private:
		Gicame::Concurrency::Impl::CircularBufferDescriptor* header;
		size_t capacity;
		std::vector<uint8_t> buffer;
		Signal dataPresentEvent;
		Signal dataFreeEvent;

	private:
		void waitElemPresent(const size_t dataSize);
		bool waitElemPresent(const size_t dataSize, uint32_t timeoutMs);
		void waitFreeSpace(const size_t dataSize);

	public:
		GICAME_API SPSCQueue(const size_t capacity_);
		GICAME_API ~SPSCQueue();
		GICAME_API void push(const void* data, size_t dataSize);
		GICAME_API void pop(void* outBuffer, size_t dataSize);
		GICAME_API bool pop(void* outBuffer, size_t dataSize, uint32_t timeoutMs);
		GICAME_API size_t size() const noexcept;
		GICAME_API size_t freeSpace() const noexcept;

		// IDataExchanger interface
		virtual size_t send(const void* data, const size_t dataSize) override final;
		virtual bool isSenderConnected() const override final;
		virtual size_t receive(void* outBuffer, const size_t dataSize) override final;
		virtual bool isReceiverConnected() const override final;

	};


	/*
	 * Inline implementation
	 */

	inline size_t SPSCQueue::send(const void* data, const size_t dataSize) {
		push(data, dataSize);
		return dataSize;
	}

	inline bool SPSCQueue::isSenderConnected() const { return true; }

	inline size_t SPSCQueue::receive(void* outBuffer, const size_t dataSize) {
		pop(outBuffer, dataSize);
		return dataSize;
	}

	inline bool SPSCQueue::isReceiverConnected() const { return true; }

};

#endif
