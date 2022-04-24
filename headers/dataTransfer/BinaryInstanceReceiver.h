#ifndef __BINARYINSTANCERECEIVER_H__
#define __BINARYINSTANCERECEIVER_H__


#include "../common.h"
#include "../interfaces/IReceiver.h"
#include "ObjectReceiver.h"


namespace Gicame {

	/**
	 * Send data to the other peer. The other peer does not have to know the data size a priori:
	 * this information is sent before the data itself.
	 */
	class BinaryInstanceReceiver {

	private:
		ObjectReceiver objectReceiver;

	public:
		BinaryInstanceReceiver(IReceiver* receiver);
		template<class DataType> DataType* receive(DataType* buffer);
		template<class DataType> DataType receive();

	};


	/*
	 * Inline implementation
	 */

	 inline BinaryInstanceReceiver::BinaryInstanceReceiver(IReceiver* receiver) : objectReceiver(receiver) {}

	 template<class DataType>
	 inline DataType* BinaryInstanceReceiver::receive(DataType* buffer) {
		 const uint32_t receivedBytes = objectReceiver.receive(buffer, sizeof(DataType));
		 if (receivedBytes != sizeof(DataType)) {
			 throw RUNTIME_ERROR("Error receiving data");
		 }
		 return buffer;
	 }

	 template<class DataType>
	 inline DataType BinaryInstanceReceiver::receive() {
		 DataType result;
		 receive(&result);
		 return result;
	 }

};


#endif
