#ifndef __RPCCLIENT_H__
#define __RPCCLIENT_H__


#include "../common.h"
#include "../interfaces/IDataExchanger.h"
#include "../dataTransfer/BinaryInstanceExchanger.h"
#include "./RpcCommon.h"
#include "./RerBuilding.h"


namespace Gicame {

    class RpcClient {

        MOVABLE_BUT_NOT_COPYABLE;

    private:
        IDataExchanger* dataExchanger;

    private:
        /**
         * Send an RpcExecutionRequest and its serialized parameters
         */
        void sendExeRequest(const RpcExecutionRequest& rer, const std::vector<std::vector<byte_t>> paramValues);

    public:
        RpcClient(IDataExchanger* dataExchanger);

        /**
         * Build a lamdas to send the RpcExecutionRequests
         */
        template <class RetType>
        auto get(const FunctionId functionId);

        /**
         * Build a lamdas to send the RpcExecutionRequests
         */
        template <class RetType, class... Args>
        inline auto RpcClient::get(const FunctionId functionId, const Args... args);

    };


    /*
     * Inline implementation
     */

    inline RpcClient::RpcClient(IDataExchanger* dataExchanger) : dataExchanger(dataExchanger)
    {
        // Nothing here
    }

    inline void RpcClient::sendExeRequest(const RpcExecutionRequest& rer, const std::vector<std::vector<byte_t>> paramValues) {
        // Compute dimensions
        const size_t paramCount = paramValues.size();
        size_t totalParamsSize = 0;
        for (size_t paramIndex = 0; paramIndex < paramCount; ++paramIndex)
            totalParamsSize += paramValues[paramIndex].size();
        
        // Accumulate all paramValues into a single buffer to send all of them together
        std::vector<byte_t> sendingBuffer(totalParamsSize);
        byte_t* sendingBufferPtr = sendingBuffer.data();
        for (size_t paramIndex = 0; paramIndex < paramCount; ++paramIndex) {
            for (size_t i = 0; i < paramValues[paramIndex].size(); ++i, ++sendingBufferPtr)
                *sendingBufferPtr = paramValues[paramIndex][i];
        }

        // Data exchange
        dataExchanger->send(rer.serialize());
        if (!sendingBuffer.empty())
            dataExchanger->send(sendingBuffer);
    }

    template <class RetType>
    inline auto RpcClient::get(const FunctionId functionId) {
        return [&]() {
            RpcExecutionRequest rer(functionId, 0);
            sendExeRequest(rer, {});
        };
    }

    template <class RetType, class... Args>
    inline auto RpcClient::get(const FunctionId functionId, const Args... args) {
        const RpcExecutionRequest rer = Gicame::RerBuilding::build(functionId, args...);
        return [](const Args... args) {
            sendExeRequest(rer, {});  // WIP
        };
    }

};


#endif
