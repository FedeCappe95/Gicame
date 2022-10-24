#include "../headers/rpc/RpcClient.h"


using namespace Gicame;


RpcClient::RpcClient(IDataExchanger* dataExchanger) : dataExchanger(dataExchanger) {
    // Nothing here
}

uint64_t RpcClient::sendExeRequest(const RpcExecutionRequest& rer, const std::vector<byte_t> paramsDump) {
    // Data exchange
    dataExchanger->send(rer.serialize());
    if (!paramsDump.empty())
        dataExchanger->send(paramsDump);

    // Receive result
    const std::vector<byte_t> reply = dataExchanger->receive(sizeof(uint64_t));
    return *((uint64_t*)reply.data());
}
