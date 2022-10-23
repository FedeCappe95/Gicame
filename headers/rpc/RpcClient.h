#ifndef __RPCCLIENT_H__
#define __RPCCLIENT_H__


#include "../common.h"
#include "../interfaces/IDataExchanger.h"
#include "../dataTransfer/BinaryInstanceExchanger.h"
#include "./RpcCommon.h"
#include "./RerBuilding.h"


namespace Gicame {

    class GICAME_API RpcClient {

        MOVABLE_BUT_NOT_COPYABLE;

    private:
        IDataExchanger* dataExchanger;

    private:
        /**
         * Send an RpcExecutionRequest and its serialized parameters
         */
        uint64_t sendExeRequest(const RpcExecutionRequest& rer, const std::vector<std::vector<byte_t>> paramValues);

    public:
        RpcClient(IDataExchanger* dataExchanger);

        /**
         * Build a lamdas to send the RpcExecutionRequests
         */
        template <class RetType, class... Args>
        auto get(const FunctionId functionId);

    };


    /*
     * Inline implementation
     */

    template <class RetType, class... Args>
    inline auto RpcClient::get(const FunctionId functionId) {
        RpcClient* thisRpcClient = this;
        return [=](const Args... args) {
            const RpcExecutionRequest rer = Gicame::RerBuilding::build(functionId, args...);
            if (rer.paramCount == 2) {  // Patch WIP
                const uint64_t result = thisRpcClient->sendExeRequest(rer, { {0x01, 0x00, 0x00, 0x00}, {0x02, 0x00, 0x00, 0x00} });  // WIP
                return (RetType)result;
            }
            const uint64_t result = thisRpcClient->sendExeRequest(rer, {});
            return (RetType)result;
        };
    }

};


#endif
