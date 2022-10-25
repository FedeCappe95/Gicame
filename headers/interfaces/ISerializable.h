#ifndef __ISERIALIZABLE_H__
#define __ISERIALIZABLE_H__


#include <vector>
#include "../common.h"


namespace Gicame {

    class GICAME_API ISerializable {

    public:
        virtual std::vector<byte_t> serialize() const = 0;

    };

};


#endif
