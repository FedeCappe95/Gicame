#ifndef __IRECEIVER_H__
#define __IRECEIVER_H__


#include "../common.h"
#include <vector>


namespace Gicame {

    class GICAME_API IReceiver {

    public:
        static constexpr uint32_t RECEIVER_MAX_SIZE = (uint32_t)INT32_MAX;

    public:
        virtual uint32_t receive(void* buffer, const uint32_t size);
        virtual std::vector<byte_t> receive(const uint32_t size);

    };

    inline std::vector<byte_t> IReceiver::receive(const uint32_t size) {
        if (size > IReceiver::RECEIVER_MAX_SIZE) {
            throw RUNTIME_ERROR("Receiving too much");
        }

        std::vector<byte_t> ret(size, 0u);
        uint32_t receivedBytes = receive(ret.data(), size);
        ret.resize(receivedBytes);
        return ret;
    }

};


#endif
