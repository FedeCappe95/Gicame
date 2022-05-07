#ifndef __OBJECTRECEIVER_H__
#define __OBJECTRECEIVER_H__


#include "../common.h"
#include "../interfaces/IReceiver.h"


namespace Gicame {

	/**
	 * Receive data from the other peer. ObjectReceiver does not have to know the data size a
	 * priori: this information is sent before the data itself.
	 */
	class ObjectReceiver : public IReceiver {

	private:
		IReceiver* receiver;

	public:
		ObjectReceiver(IReceiver* receiver);
		virtual uint32_t receive(void* buffer, const uint32_t size);
        virtual std::vector<byte_t> receive(const uint32_t maxSize);

	};


	/*
	 * Inline implementation
	 */

	 inline ObjectReceiver::ObjectReceiver(IReceiver* receiver) : receiver(receiver) {}

	 inline uint32_t ObjectReceiver::receive(void* buffer, const uint32_t size) {
		 uint32_t incomingObjectSize;
		 uint32_t receivedBytes = receiver->receive(&incomingObjectSize, sizeof(incomingObjectSize));

		 if (unlikely(receivedBytes != sizeof(incomingObjectSize))) {
			 throw RUNTIME_ERROR("Error receiving the incoming object size");
		 }

		 if (unlikely(incomingObjectSize > IReceiver::RECEIVER_MAX_SIZE)) {
             throw RUNTIME_ERROR("Receiving too much");
         }

		 if (unlikely(incomingObjectSize > size)) {
			 throw RUNTIME_ERROR("Incoming object size greater then buffer size");
		 }

		 receivedBytes = 0;
		 do {
			 receivedBytes += receiver->receive((byte_t*)buffer + receivedBytes, incomingObjectSize - receivedBytes);
		 } while (receivedBytes < incomingObjectSize);

		 return receivedBytes;
	 }

	 inline std::vector<byte_t> ObjectReceiver::receive(const uint32_t maxSize) {
         std::vector<byte_t> ret(maxSize, 0u);
         uint32_t receivedBytes = receive(ret.data(), maxSize);
         ret.resize(receivedBytes);
         return ret;
	 }

};


#endif
