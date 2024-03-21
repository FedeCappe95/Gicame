#ifndef __GICAME__CONCURRENCY__IMPLEMENTATION_DETAILS__META_H__
#define __GICAME__CONCURRENCY__IMPLEMENTATION_DETAILS__META_H__


#include "../../common.h"
#include <atomic>


namespace Gicame::Concurrency::Impl {

	struct SPSCQueueMeta {
		std::atomic<size_t> head;
		std::atomic<size_t> tail;
	};

};

#endif
