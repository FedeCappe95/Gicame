#include "rpc/RpcServer.h"


using namespace Gicame;


RpcServer::RpcServer(IDataExchanger* dataExchanger) :
    dataExchanger(dataExchanger)
{
    // Default InvalidRequestEventHandler
    invalidRequestEventHandler = [&](const RpcExecutionRequest& rer, const InvalidRequestReason reason) {
        switch (reason) {
        case InvalidRequestReason::INVALID_FUNCTION_ID:
            throw RUNTIME_ERROR("Invalid request: invalid functionId");
        case InvalidRequestReason::BAD_PARAMETERS:
            throw RUNTIME_ERROR("Invalid request: bad parameters");
        case InvalidRequestReason::RER_INTEGRITY_CHECK_FAILED:
            throw RUNTIME_ERROR("Invalid request: failed integrity check");
        default:
            throw RUNTIME_ERROR("Invalid request");
        }
        return false;
    };
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
        return invalidRequestEventHandler(exeRequest, InvalidRequestReason::RER_INTEGRITY_CHECK_FAILED);
    }

    // Find the RpcFunctionDescriptor
    auto funStoreIter = funStore.find(exeRequest.functionId);
    if (unlikely(funStoreIter == funStore.end())) {
        return invalidRequestEventHandler(exeRequest, InvalidRequestReason::INVALID_FUNCTION_ID);
    }
    RpcFunctionDescriptor& fd = funStoreIter->second;

    // Receive parameters
    std::vector<std::vector<byte_t>> params(exeRequest.paramCount);
    size_t totalParametersSize = 0;
    for (size_t paramIndex = 0; paramIndex < exeRequest.paramCount; ++paramIndex)
        totalParametersSize += exeRequest.params[paramIndex].size;
    if (totalParametersSize > 0) {
        const std::vector<byte_t> allParametersBuffer = dataExchanger->receive((uint32_t)totalParametersSize);
        if (allParametersBuffer.size() != totalParametersSize) {
            return invalidRequestEventHandler(exeRequest, InvalidRequestReason::BAD_PARAMETERS);
        }
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

void RpcServer::setInvalidRequestEventHandler(InvalidRequestEventHandler& handler) {
    invalidRequestEventHandler = handler;
}
