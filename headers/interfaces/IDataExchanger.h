#ifndef __IDATAEXCHANGER_H__
#define __IDATAEXCHANGER_H__


#include "../common.h"
#include "./IReceiver.h"
#include "./ISender.h"


namespace Gicame {

    class GICAME_API IDataExchanger : public IReceiver, public ISender {
    
    public:
        virtual bool isConnected() const = 0;

    };

};


#endif
