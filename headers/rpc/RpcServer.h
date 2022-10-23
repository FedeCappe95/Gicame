#ifndef __RPCSERVER_H__
#define __RPCSERVER_H__


#include <unordered_map>
#include <functional>
#include <vector>
#include "../common.h"
#include "../interfaces/IDataExchanger.h"
#include "../dataTransfer/BinaryInstanceExchanger.h"
#include "./RpcCommon.h"


namespace Gicame {

    class GICAME_API RpcServer {

        MOVABLE_BUT_NOT_COPYABLE;

    private:
        /**
         * Describe an RPC Function stored in the RpcServer
         */
        struct RpcFunctionDescriptor {
            RpcFunction rpcFunction;
            FunctionId functionId;
            inline RpcFunctionDescriptor(const RpcFunction rpcFunction, const FunctionId functionId) :
                rpcFunction(rpcFunction), functionId(functionId) {}
        };

    private:
#ifdef MSVC
#pragma warning(push)
#pragma warning(disable:4251)
#endif
        IDataExchanger* dataExchanger;
        std::unordered_map<FunctionId, RpcFunctionDescriptor> funStore;
#ifdef MSVC
#pragma warning(pop)
#endif

    public:
        RpcServer(IDataExchanger* dataExchanger);

        /**
         * Add or replace a function to the RpcServer
         */
        void registerFunction(RpcFunction rpcFunction, const FunctionId functionId);

        /**
         * Process a single execution request
         */
        bool oneShot();

        /**
         * Process execution requests in loop
         */
        void run();

    };

};


#endif
