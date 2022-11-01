#ifndef __GICAME__BINARYINSTANCESENDER_H__
#define __GICAME__BINARYINSTANCESENDER_H__


#include "../common.h"
#include "../interfaces/ISender.h"
#include "./ObjectSender.h"


namespace Gicame {

	/**
	 * Send data to the other peer. The other peer does not have to know the data size a priori:
	 * this information is sent before the data itself.
	 */
	class BinaryInstanceSender {

	private:
		ObjectSender objectSender;

	public:
		BinaryInstanceSender(ISender* sender);
		template<class DataType> void send(const DataType* data);

	};


	/*
	 * Inline implementation
	 */

	 inline BinaryInstanceSender::BinaryInstanceSender(ISender* sender) : objectSender(sender) {}

	 template<class DataType>
	 inline void BinaryInstanceSender::send(const DataType* data) {
		 const uint32_t sent = objectSender.send(data, sizeof(DataType));
		 if (sent != sizeof(DataType)) {
			 throw RUNTIME_ERROR("Error sending data");
		 }
	 }

};


#endif
