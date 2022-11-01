#ifndef __GICAME__RPCCLIENT_H__
#define __GICAME__RPCCLIENT_H__


#include "../common.h"
#include "../interfaces/IDataExchanger.h"
#include "../dataTransfer/BinaryInstanceExchanger.h"
#include "./RpcCommon.h"
#include "./RerBuilding.h"
#include "../dataSerialization/BinarySerializer.h"


namespace Gicame {

    class GICAME_API RpcClient {

        MOVABLE_BUT_NOT_COPYABLE;

    private:
        IDataExchanger* dataExchanger;
        BinarySerializer serializer;

    private:
        /**
         * Send an RpcExecutionRequest and its serialized parameters
         */
        uint64_t sendExeRequest(const RpcExecutionRequest& rer, const std::vector<byte_t> paramsDump);

    public:
        RpcClient(IDataExchanger* dataExchanger, const BinarySerializer& serializer = BinarySerializer());

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
            auto [rer, paramsDump] = Gicame::RerBuilding::build(serializer, functionId, args...);
            const uint64_t result = thisRpcClient->sendExeRequest(rer, paramsDump);
            return (RetType)result;
        };
    }

};


#endif
