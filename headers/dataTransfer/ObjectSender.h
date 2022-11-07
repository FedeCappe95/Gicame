#ifndef __GICAME__OBJECTSENDER_H__
#define __GICAME__OBJECTSENDER_H__


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
		virtual size_t send(const void* buffer, const size_t size) override;
		virtual size_t send(const std::vector<byte_t>& buffer) override;
		virtual bool isSenderConnected() const override;

	};


	/*
	 * Inline implementation
	 */

	 inline ObjectSender::ObjectSender(ISender* sender) : sender(sender) {}

	 inline size_t ObjectSender::send(const void* buffer, const size_t size) {
		 const uint64_t sizeToSend = (uint64_t)size;
		 size_t sentBytes = sender->send(&sizeToSend, sizeof(sizeToSend));
		 if (unlikely(sentBytes != sizeof(size))) {
			 throw RUNTIME_ERROR("Error sending outgoing object size");
		 }

		 sentBytes = 0;
		 do {
			 sentBytes += sender->send((byte_t*)buffer + sentBytes, size - sentBytes);
		 } while(sentBytes < size);

		 return sentBytes;
	 }

	 inline size_t ObjectSender::send(const std::vector<byte_t>& buffer) {
		 return send(buffer.data(), (uint32_t)buffer.size());
	 }

	 inline bool ObjectSender::isSenderConnected() const {
		 return sender->isSenderConnected();
	 }

};


#endif
