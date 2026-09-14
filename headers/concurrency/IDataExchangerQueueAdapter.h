#ifndef __GICAME__CONCURRENCY__IDATAEXCHANGERQUEUEADAPETER_H__
#define __GICAME__CONCURRENCY__IDATAEXCHANGERQUEUEADAPETER_H__


#include "../common.h"
#include "../interfaces/IDataExchanger.h"


namespace Gicame::Concurrency {

	template <typename Queue>
	class IDataExchangerQueueAdapter : public IDataExchanger {

	public:
		virtual size_t send(const void* data, const size_t dataSize) override final;
		virtual bool isSenderConnected() const override final;
		virtual size_t receive(void* outBuffer, const size_t dataSize) override final;
		virtual bool isReceiverConnected() const override final;

	};


	/*
	 * Inline implementation
	 */

	template <typename Queue>
	inline size_t IDataExchangerQueueAdapter<Queue>::send(const void* data, const size_t dataSize) {
		static_cast<Queue*>(this)->push(data, dataSize);
		return dataSize;
	}

	template <typename Queue>
	inline bool IDataExchangerQueueAdapter<Queue>::isSenderConnected() const { return true; }

	template <typename Queue>
	inline size_t IDataExchangerQueueAdapter<Queue>::receive(void* outBuffer, const size_t dataSize) {
		static_cast<Queue*>(this)->pop(outBuffer, dataSize);
		return dataSize;
	}

	template <typename Queue>
	inline bool IDataExchangerQueueAdapter<Queue>::isReceiverConnected() const { return true; }

};

#endif
