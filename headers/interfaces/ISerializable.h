#ifndef __ISERIALIZABLE_H__
#define __ISERIALIZABLE_H__


#include "../common.h"
#include <vector>


namespace Gicame {

    class ISerializable {

    public:
        virtual std::vector<byte_t> serialize() = 0;

    };

};


#endif
