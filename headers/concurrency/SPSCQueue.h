#ifndef __GICAME__CONCURRENCY__SPSPQUEUE_H__
#define __GICAME__CONCURRENCY__SPSPQUEUE_H__


#include "../common.h"
#include "../utils/NotCopyable.h"


// Forward declarations
namespace Gicame::Concurrency::Impl {
	struct SPSCQueueMeta;
}


namespace Gicame::Concurrency {

	/**
	 * @brief Abstract class for a Single Producer Single Consumer Queue
	 *
	 * Implementation: circular buffer
	 */
	class SPSCQueue {

		NOT_COPYABLE(SPSCQueue)

	public:    // Public types
		enum class BufferWrappingStrategy {
			MASTER, SLAVE
		};

	protected:  // Protected data members
		Impl::SPSCQueueMeta* meta;
		uint8_t* const buffer;
		const size_t capacity;

	protected:  // Protected data members
		virtual void waitElemPresent(const size_t dataSize) = 0;
		virtual void waitFreeSpace(const size_t dataSize) = 0;

	public:    // Public methods
		GICAME_API SPSCQueue(void* buffer, const size_t capacity, const BufferWrappingStrategy bws);
		GICAME_API virtual ~SPSCQueue();
		GICAME_API virtual void push(const void* data, const size_t dataSize);
		GICAME_API virtual void pop(void* outBuffer, const size_t dataSize);
		GICAME_API virtual size_t size();

		template<typename Type>
		void push(const Type& obj);

		template<typename Type>
		Type pop();

	};


	/*
	 * Inline implementation
	 */

	template<typename Type>
	inline void SPSCQueue::push(const Type& obj) {
		// TODO: primitive types only are allowed
		push(&obj, sizeof(obj));
	}

	template<typename Type>
	inline Type SPSCQueue::pop() {
		// TODO: primitive types only are allowed
		Type obj;
		pop(&obj, sizeof(obj));
		return obj;
	}

};

#endif
