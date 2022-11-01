#ifndef __GICAME__IRECEIVER_H__
#define __GICAME__IRECEIVER_H__


#include <vector>
#include "../common.h"


namespace Gicame {

    class GICAME_API IReceiver {

    public:
        static constexpr uint32_t RECEIVER_MAX_SIZE = (uint32_t)INT32_MAX;

    public:
        virtual uint32_t receive(void* buffer, const uint32_t size) = 0;
        virtual std::vector<byte_t> receive(const uint32_t size);
        virtual bool isReceiverConnected() const = 0;

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
