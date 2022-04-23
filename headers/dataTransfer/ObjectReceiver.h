#ifndef __OBJECTRECEIVER_H__
#define __OBJECTRECEIVER_H__


#include "../common.h"
#include "../interfaces/IReceiver.h"


namespace Gicame {

	class GICAME_API ObjectReceiver : public IReceiver {

	private:
		IReceiver* receiver;

	public:
		ObjectSender(IReceiver* receiver);
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

		 if (receivedBytes != sizeof(incomingObjectSize)) {
			 throw RUNTIME_ERROR("Error receiving the incoming object size");
		 }

		 if (incomingObjectSize > IReceiver::RECEIVER_MAX_SIZE) {
             throw RUNTIME_ERROR("Receiving too much");
         }

		 if (incomingObjectSize > size) {
			 throw RUNTIME_ERROR("Incoming object size greater then buffer size");
		 }

		 return receiver->receive(buffer, incomingObjectSize);
	 }

	 inline std::vector<byte_t> ObjectReceiver::receive(const uint32_t maxSize) {
         std::vector<byte_t> ret(maxSize, 0u);
         uint32_t receivedBytes = receive(ret.data(), size);
         ret.resize(receivedBytes);
         return ret;
	 }

};


#endif
