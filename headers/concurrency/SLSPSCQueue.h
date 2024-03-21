#ifndef __GICAME__CONCURRENCY__SLSPSPQUEUE_H__
#define __GICAME__CONCURRENCY__SLSPSPQUEUE_H__


#include "../common.h"
#include "./SPSCQueue.h"


// Help-functions usefull in some classes
namespace Gicame::Concurrency {

	/**
	 * @brief Implementation of SPSCQueue via spinlocks
	 */
	class SLSPSCQueue : public SPSCQueue {

		MOVABLE_BUT_NOT_COPYABLE;

	protected:
		GICAME_API virtual void waitElemPresent(const size_t dataSize) override final;
		GICAME_API virtual void waitFreeSpace(const size_t dataSize) override final;

	public:
		GICAME_API SLSPSCQueue(void* buffer, const size_t capacity, const BufferWrappingStrategy bws);
		GICAME_API virtual ~SLSPSCQueue() override final;

	};

};

#endif
