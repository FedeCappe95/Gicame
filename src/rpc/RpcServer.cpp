#include "rpc/RpcServer.h"


using namespace Gicame;


RpcServer::RpcServer(IDataExchanger* dataExchanger) :
    dataExchanger(dataExchanger)
{
    // Nothing here
}

void RpcServer::registerFunction(RpcFunction rpcFunction, const FunctionId functionId) {
    funStore.insert_or_assign(functionId, RpcFunctionDescriptor(rpcFunction, functionId));
}

bool RpcServer::oneShot() {
    // Receive the RpcExecutionRequest
    const size_t exeRequestSerSize = RpcExecutionRequest().serialize().size();  // Worst method possible, must be changed, but it works for now...
    const std::vector<byte_t> serExeRequest = dataExchanger->receive((uint32_t)exeRequestSerSize);
    if (serExeRequest.empty())
        return false;
    RpcExecutionRequest exeRequest = RpcExecutionRequest::deserialize(serExeRequest);
    if (unlikely(!exeRequest.checkIntegrity())) {
        throw RUNTIME_ERROR("Invalid exeRequest: failed integrity check");
    }

    // Find the RpcFunctionDescriptor
    auto funStoreIter = funStore.find(exeRequest.functionId);
    if (unlikely(funStoreIter == funStore.end())) {
        throw RUNTIME_ERROR("Invalid exeRequest: invalid functionId");
    }
    RpcFunctionDescriptor& fd = funStoreIter->second;

    // Receive parameters
    std::vector<std::vector<byte_t>> params(exeRequest.paramCount);
    size_t totalParametersSize = 0;
    for (size_t paramIndex = 0; paramIndex < exeRequest.paramCount; ++paramIndex)
        totalParametersSize += exeRequest.params[paramIndex].size;
    if (totalParametersSize > 0) {
        const std::vector<byte_t> allParametersBuffer = dataExchanger->receive((uint32_t)totalParametersSize);
        const byte_t* ptr = allParametersBuffer.data();
        for (size_t paramIndex = 0; paramIndex < exeRequest.paramCount; ++paramIndex) {
            params[paramIndex] = std::vector<byte_t>(exeRequest.params[paramIndex].size);
            for (size_t i = 0; i < exeRequest.params[paramIndex].size; ++i) {
                params[paramIndex][i] = *ptr;
                ++ptr;
            }
        }
    }

    // Invoke function
    const uint64_t result = fd.rpcFunction(&exeRequest, params);

    // Send result
    dataExchanger->send(&result, sizeof(uint64_t));

    return true;
}

void RpcServer::run() {
    while (dataExchanger->isConnected()) {
        const bool connectionStatus = oneShot();
        if (!connectionStatus)
            break;
    }
}
