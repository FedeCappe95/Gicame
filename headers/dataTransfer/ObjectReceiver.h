#ifndef __GICAME__OBJECTRECEIVER_H__
#define __GICAME__OBJECTRECEIVER_H__


#include "../common.h"
#include "../interfaces/IReceiver.h"


namespace Gicame {

	/**
	 * Receive data from the other peer. ObjectReceiver does not have to know the data bufferSize a
	 * priori: this information is sent before the data itself.
	 */
	class ObjectReceiver {

	private:
		IReceiver* receiver;

	public:
		ObjectReceiver(IReceiver* receiver);
		virtual ~ObjectReceiver() = default;
		size_t receiveObject(void* buffer, const size_t bufferSize);
		bool isReceiverConnected() const;

	};


	/*
	 * Inline implementation
	 */

	 inline ObjectReceiver::ObjectReceiver(IReceiver* receiver) : receiver(receiver) {}

	 inline size_t ObjectReceiver::receiveObject(void* buffer, const size_t bufferSize) {
		 uint64_t incomingObjectSize;
		 size_t receivedBytes = receiver->receive(&incomingObjectSize, sizeof(incomingObjectSize));

		 if (unlikely(receivedBytes != sizeof(incomingObjectSize))) {
			 throw RUNTIME_ERROR("Error receiving the incoming object size");
		 }

		 if (unlikely(incomingObjectSize > bufferSize))
			 throw RUNTIME_ERROR("Incoming object size greater then buffer size");

		 receivedBytes = 0;
		 do {
			 receivedBytes += receiver->receive((byte_t*)buffer + receivedBytes, (size_t)incomingObjectSize - receivedBytes);
		 } while (receivedBytes < incomingObjectSize);

		 return receivedBytes;
	 }

	 inline bool ObjectReceiver::isReceiverConnected() const {
		 return receiver->isReceiverConnected();
	 }

};


#endif
