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
        auto get(const FunctionId functionId, const Args... args);

    };


    /*
     * Inline implementation
     */

    template <class RetType>
    inline auto RpcClient::get(const FunctionId functionId) {
        return [&]() {
            RpcExecutionRequest rer(functionId, 0);
            sendExeRequest(rer, {});
            // TODO: read reply;
            RetType ret;
            return ret;
        };
    }

    template <class RetType, class... Args>
    inline auto RpcClient::get(const FunctionId functionId, const Args... args) {
        const RpcExecutionRequest rer = Gicame::RerBuilding::build(functionId, args...);
        return [](const Args... args) {
            sendExeRequest(rer, {});  // WIP
            // TODO: read reply;
            RetType ret;
            return ret;
        };
    }

};


#endif
