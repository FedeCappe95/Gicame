#ifndef __GICAME__OBJECTRECEIVER_H__
#define __GICAME__OBJECTRECEIVER_H__


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
		virtual size_t receive(void* buffer, const size_t size) override;
        virtual std::vector<byte_t> receive(const size_t maxSize) override;
		virtual bool isReceiverConnected() const override;

	};


	/*
	 * Inline implementation
	 */

	 inline ObjectReceiver::ObjectReceiver(IReceiver* receiver) : receiver(receiver) {}

	 inline size_t ObjectReceiver::receive(void* buffer, const size_t size) {
		 uint64_t incomingObjectSize;
		 size_t receivedBytes = receiver->receive(&incomingObjectSize, sizeof(incomingObjectSize));

		 if (unlikely(receivedBytes != sizeof(incomingObjectSize))) {
			 throw RUNTIME_ERROR("Error receiving the incoming object size");
		 }

		 if (unlikely(incomingObjectSize > size)) {
			 throw RUNTIME_ERROR("Incoming object size greater then buffer size");
		 }

		 receivedBytes = 0;
		 do {
			 receivedBytes += receiver->receive((byte_t*)buffer + receivedBytes, (size_t)incomingObjectSize - receivedBytes);
		 } while (receivedBytes < incomingObjectSize);

		 return receivedBytes;
	 }

	 inline std::vector<byte_t> ObjectReceiver::receive(const size_t maxSize) {
         std::vector<byte_t> ret(maxSize, 0u);
		 const size_t receivedBytes = receive(ret.data(), maxSize);
         ret.resize(receivedBytes);
         return ret;
	 }

	 inline bool ObjectReceiver::isReceiverConnected() const {
		 return receiver->isReceiverConnected();
	 }

};


#endif
