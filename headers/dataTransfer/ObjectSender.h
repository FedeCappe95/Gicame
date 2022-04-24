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
		virtual uint32_t send(const void* buffer, const uint32_t size);
		virtual uint32_t send(const std::vector<byte_t>& buffer);

	};


	/*
	 * Inline implementation
	 */

	 inline ObjectSender::ObjectSender(ISender* sender) : sender(sender) {}

	 inline uint32_t ObjectSender::send(const void* buffer, const uint32_t size) {
		 if (size > ISender::SENDER_MAX_SIZE) {
             throw RUNTIME_ERROR("Sending too much!");
         }
		 sender->send(&size, sizeof(size));
		 sender->send(buffer, size);
	 }

	 inline uint32_t ObjectSender::send(const std::vector<byte_t>& buffer) {
         return send(buffer.data(), buffer.size());
	 }

};


#endif
