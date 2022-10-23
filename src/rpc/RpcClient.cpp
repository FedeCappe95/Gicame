#include "../headers/rpc/RpcClient.h"


using namespace Gicame;


RpcClient::RpcClient(IDataExchanger* dataExchanger) : dataExchanger(dataExchanger) {
    // Nothing here
}

uint64_t RpcClient::sendExeRequest(const RpcExecutionRequest& rer, const std::vector<std::vector<byte_t>> paramValues) {
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

    // Receive result
    const std::vector<byte_t> reply = dataExchanger->receive(sizeof(uint64_t));
    return *((uint64_t*)reply.data());
}
