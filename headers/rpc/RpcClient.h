#ifndef __RPCCLIENT_H__
#define __RPCCLIENT_H__


#include <unordered_map>
#include <functional>
#include "../common.h"
#include "../interfaces/IDataExchanger.h"
#include "../dataTransfer/BinaryInstanceExchanger.h"
#include "RpcCommon.h"


namespace Gicame {

    class RpcClient {

        MOVABLE_BUT_NOT_COPYABLE;

    private:
        IDataExchanger* dataExchanger;

    private:
        void sendExeRequest(const FunctionId functionId, const std::vector<RpcParamDescriptor> params, const std::vector<void*> paramValues);

    public:
        RpcClient(IDataExchanger* dataExchanger);

        /**
         * Template functions to retrieve lamdas to send the RpcExecutionRequests
         */
        template <class RetType>
        auto get(const FunctionId functionId);
        template <class RetType, class Arg0Type>
        auto get(const FunctionId functionId);
        template <class RetType, class Arg0Type, class Arg1Type>
        auto get(const FunctionId functionId);
        template <class RetType, class Arg0Type, class Arg1Type, class Arg2Type>
        auto get(const FunctionId functionId);
        template <class RetType, class Arg0Type, class Arg1Type, class Arg2Type, class Arg3Type>
        auto get(const FunctionId functionId);
        template <class RetType, class Arg0Type, class Arg1Type, class Arg2Type, class Arg3Type, class Arg4Type>
        auto get(const FunctionId functionId);
        template <class RetType, class Arg0Type, class Arg1Type, class Arg2Type, class Arg3Type, class Arg4Type, class Arg5Type>
        auto get(const FunctionId functionId);

    };


    /*
     * Inline implementation
     */

    inline RpcClient::RpcClient(IDataExchanger* dataExchanger) : dataExchanger(dataExchanger)
    {
        // Nothing here
    }

    inline void RpcClient::sendExeRequest(
        const FunctionId functionId, const std::vector<RpcParamDescriptor> params, const std::vector<void*> paramValues
    ) {
        if (unlikely(params.size() != paramValues.size())) {
            throw RUNTIME_ERROR("Invalid couple of arguments params and paramValues");
        }

        uint32_t paramCount = (uint32_t)params.size();

        std::vector<byte_t> paramSerializedBuffer;
        RpcExecutionRequest rer(functionId, paramCount);

        for (uint32_t paramIndex = 0; paramIndex < paramCount; ++paramIndex) {
            rer.params[paramIndex] = params[paramIndex];
            byte_t* paramPointer = (byte_t*)paramValues[paramIndex];
            for (size_t i = 0; i < rer.params[paramIndex].size; ++i) {
                paramSerializedBuffer.emplace_back(*paramPointer);
                ++paramPointer;
            }
        }

        dataExchanger->send(rer.serialize());

        if (!paramSerializedBuffer.empty())
            dataExchanger->send(paramSerializedBuffer);
    }

    template <class RetType>
    inline auto RpcClient::get(const FunctionId functionId) {
        return [&]() {
            sendExeRequest(functionId, std::vector<RpcParamDescriptor>(), {});
        };
    }

    template <class RetType, class Arg0Type>
    inline auto RpcClient::get(const FunctionId functionId) {
        return [&](Arg0Type arg0) {
            sendExeRequest(functionId, { RpcParamDescriptor(sizeof(Arg0Type)) }, { &arg0 });
        };
    }

    template <class RetType, class Arg0Type, class Arg1Type>
    inline auto RpcClient::get(const FunctionId functionId) {
        return [&](Arg0Type arg0, Arg1Type arg1) {
            sendExeRequest(
                functionId,
                { RpcParamDescriptor(sizeof(Arg0Type)), RpcParamDescriptor(sizeof(Arg1Type)) },
                { &arg0, &arg1 }
            );
        };
    }

    template <class RetType, class Arg0Type, class Arg1Type, class Arg2Type>
    inline auto RpcClient::get(const FunctionId functionId) {
        return [&](Arg0Type arg0, Arg1Type arg1, Arg2Type arg2) {
            sendExeRequest(
                functionId,
                { RpcParamDescriptor(sizeof(Arg0Type)), RpcParamDescriptor(sizeof(Arg1Type)), RpcParamDescriptor(sizeof(Arg2Type)) },
                { &arg0, &arg1, &arg2 }
            );
        };
    }

    template <class RetType, class Arg0Type, class Arg1Type, class Arg2Type, class Arg3Type>
    inline auto RpcClient::get(const FunctionId functionId) {
        return [&](Arg0Type arg0, Arg1Type arg1, Arg2Type arg2, Arg3Type arg3) {
            sendExeRequest(
                functionId,
                { RpcParamDescriptor(sizeof(Arg0Type)), RpcParamDescriptor(sizeof(Arg1Type)), RpcParamDescriptor(sizeof(Arg2Type)),
                  RpcParamDescriptor(sizeof(Arg3Type)) },
                { &arg0, &arg1, &arg2, &arg3 }
            );
        };
    }

    template <class RetType, class Arg0Type, class Arg1Type, class Arg2Type, class Arg3Type, class Arg4Type>
    inline auto RpcClient::get(const FunctionId functionId) {
        return [&](Arg0Type arg0, Arg1Type arg1, Arg2Type arg2, Arg3Type arg3, Arg4Type arg4) {
            sendExeRequest(
                functionId,
                { RpcParamDescriptor(sizeof(Arg0Type)), RpcParamDescriptor(sizeof(Arg1Type)), RpcParamDescriptor(sizeof(Arg2Type)),
                  RpcParamDescriptor(sizeof(Arg3Type)), RpcParamDescriptor(sizeof(Arg4Type)) },
                { &arg0, &arg1, &arg2, &arg3, &arg4 }
            );
        };
    }

    template <class RetType, class Arg0Type, class Arg1Type, class Arg2Type, class Arg3Type, class Arg4Type, class Arg5Type>
    inline auto RpcClient::get(const FunctionId functionId) {
        return [&](Arg0Type arg0, Arg1Type arg1, Arg2Type arg2, Arg3Type arg3, Arg4Type arg4, Arg5Type arg5) {
            sendExeRequest(
                functionId,
                { RpcParamDescriptor(sizeof(Arg0Type)), RpcParamDescriptor(sizeof(Arg1Type)), RpcParamDescriptor(sizeof(Arg2Type)),
                  RpcParamDescriptor(sizeof(Arg3Type)), RpcParamDescriptor(sizeof(Arg4Type)), RpcParamDescriptor(sizeof(Arg5Type)) },
                { &arg0, &arg1, &arg2, &arg3, &arg4, &arg5 }
            );
        };
    }

};


#endif
