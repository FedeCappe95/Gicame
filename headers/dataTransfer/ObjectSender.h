#ifndef __OBJECTSENDER_H__
#define __OBJECTSENDER_H__


#include "../common.h"
#include "../interfaces/ISender.h"


namespace Gicame {

	/**
	 * Send data to the other peer. The other peer does not have to know the data size a priori:
	 * this information is sent before the data itself.
	 */
	class ObjectSender : public ISender {

	private:
		ISender* sender;

	public:
		ObjectSender(ISender* sender);
		virtual uint32_t send(const void* buffer, const uint32_t size) override;
		virtual uint32_t send(const std::vector<byte_t>& buffer) override;
		virtual bool isSenderConnected() const override;

	};


	/*
	 * Inline implementation
	 */

	 inline ObjectSender::ObjectSender(ISender* sender) : sender(sender) {}

	 inline uint32_t ObjectSender::send(const void* buffer, const uint32_t size) {
		 if (unlikely(size > ISender::SENDER_MAX_SIZE)) {
             throw RUNTIME_ERROR("Sending too much");
         }

		 uint32_t sentBytes = sender->send(&size, sizeof(size));
		 if (unlikely(sentBytes != sizeof(size))) {
			 throw RUNTIME_ERROR("Error sending outgoing object size");
		 }

		 sentBytes = 0;
		 do {
			 sentBytes += sender->send((byte_t*)buffer + sentBytes, size - sentBytes);
		 } while(sentBytes < size);

		 return sentBytes;
	 }

	 inline uint32_t ObjectSender::send(const std::vector<byte_t>& buffer) {
		 if (unlikely(buffer.size() > ISender::SENDER_MAX_SIZE)) {
			 throw RUNTIME_ERROR("Sending too much");
		 }
         return send(buffer.data(), (uint32_t)buffer.size());
	 }

	 inline bool ObjectSender::isSenderConnected() const {
		 return sender->isSenderConnected();
	 }

};


#endif
