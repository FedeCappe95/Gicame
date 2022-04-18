#ifndef __IWRITER_H__
#define __IWRITER_H__


#include "../common.h"
#include <vector>


namespace Gicame {

    class GICAME_API ISender {

    public:
        static constexpr uint32_t SENDER_MAX_SIZE = (uint32_t)INT32_MAX;

    public:
        virtual uint32_t send(const void* buffer, const uint32_t size);
        virtual uint32_t send(const std::vector<byte_t>& buffer);

    };

    inline uint32_t ISender::send(const std::vector<byte_t>& buffer) {
        if (buffer.size() > SENDER_MAX_SIZE) {
            throw RUNTIME_ERROR("Sending too much!");
        }
        return send(buffer.data(), buffer.size());
    }

};


#endif
