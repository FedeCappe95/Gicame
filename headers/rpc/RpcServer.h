#ifndef __RPCSERVER_H__
#define __RPCSERVER_H__


#include <unordered_map>
#include "../common.h"
#include "../interfaces/IDataExchanger.h"
#include "RpcCommon.h"


namespace Gicame {

    class RpcServer {

        MOVABLE_BUT_NOT_COPYABLE;

    private:
        IDataExchanger* dataExchanger;
        unordered_map<FunctionId, ptr_t> funStore;

    public:
        RpcServer(IDataExchanger* dataExchanger);

        /**
         * Add or replace a function to the RpcServer
         */
        template<typename FunctionType>
        void registerFunction(FunctionType* functionPtr, const FunctionId functionId);

        /**
         * Process a single execution request
         */
        void oneShot();

    };


    /*
     * Inline implementation
     */

    template<typename FunctionType>
    inline void RpcServer::registerFunction(FunctionType* functionPtr, const FunctionId functionId) {
        funStore[functionId] = (ptr_t)functionPtr;
    }

    inline void RpcServer::oneShot() {
        
    }

};


#endif
