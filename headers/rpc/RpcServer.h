#ifndef __RPCSERVER_H__
#define __RPCSERVER_H__


#include <unordered_map>
#include <functional>
#include "../common.h"
#include "../interfaces/IDataExchanger.h"
#include "../dataTransfer/BinaryInstanceExchanger.h"
#include "RpcCommon.h"


namespace Gicame {

    class RpcServer {

        MOVABLE_BUT_NOT_COPYABLE;

    private:
        struct FunctionDescriptor {
            RpcFunction rpcFunction;
            const FunctionId functionId;
            const uint32_t paramCount;
            inline FunctionDescriptor(const RpcFunction rpcFunction, const FunctionId functionId, const uint32_t paramCount) :
                rpcFunction(rpcFunction), functionId(functionId), paramCount(paramCount) {}
        };

    private:
        IDataExchanger* dataExchanger;
        BinaryInstanceExchanger* binInstExch;
        unordered_map<FunctionId, FunctionDescriptor> funStore;

    public:
        RpcServer(IDataExchanger* dataExchanger);

        /**
         * Add or replace a function to the RpcServer
         */
        void registerFunction(RpcFunction rpcFunction, const uint32_t paramCount, const FunctionId functionId);

        /**
         * Process a single execution request
         */
        void oneShot();

        /**
         * Process execution requests in loop
         */
        void run();

    };


    /*
     * Inline implementation
     */

    inline RpcServer::RpcServer(IDataExchanger* dataExchanger) :
        dataExchanger(dataExchanger),
        binInstExch(dataExchanger)
    {
        // Nothing here
    }

    inline void RpcServer::registerFunction(RpcFunction rpcFunction, const uint32_t paramCount, const FunctionId functionId) {
        FunctionDescriptor fd(rpcFunction, functionId, paramCount);
        funStore[functionId] = fd;
    }

    inline void RpcServer::oneShot() {
        RpcExecutionRequest exeRequest = binInstExch->receive<RpcExecutionRequest>();
        if (unlikely(!exeRequest.checkIntegrity())) {
            throw RUNTIME_ERROR("Invalid exeRequest: failed integrity check");
        }

        auto funStoreIter = funStore.find(exeRequest.functionId);
        if (unlikely(funStoreIter == funStore.end())) {
            throw RUNTIME_ERROR("Invalid exeRequest: invalid functionId");
        }

        FunctionDescriptor& fd = funStoreIter.second;
        if (unlikely(fd.paramCount != exeRequest.paramCount)) {
            throw RUNTIME_ERROR("Invalid exeRequest: wrong paramCount");
        }

        fd.rpcFunction(&exeRequest);
    }

    inline void RpcServer::run() {
        while (true) {
            oneShot();
        }
    }

};


#endif
