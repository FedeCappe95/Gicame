#include <cassert>
#include "rpc/RpcServer.h"


using namespace Gicame;


RpcServer::RpcServer(IDataExchanger* dataExchanger, const BinarySerializer& serializer) :
    dataExchanger(dataExchanger),
    serializer(serializer)
{
    // Default InvalidRequestEventHandler
    invalidRequestEventHandler = [&](const RpcExecutionRequest&, const InvalidRequestReason reason) {
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

void RpcServer::registerRpcFunction(RpcFunction rpcFunction, const FunctionId functionId) {
    funStore.insert_or_assign(functionId, RpcFunctionDescriptor(rpcFunction, functionId));
}

void RpcServer::registerFunction(std::function<void()> function, const FunctionId functionId) {
    registerRpcFunction(
        [=](RpcExecutionRequest*, const std::vector<byte_t>&) {
            function();
            return 0;
        },
        functionId
    );
}

void RpcServer::registerFunction(std::function<uint64_t()> function, const FunctionId functionId) {
    registerRpcFunction(
        [=](RpcExecutionRequest*, const std::vector<byte_t>&) {
            return function();
        },
        functionId
    );
}

bool RpcServer::oneShot() {
    // Receive the RpcExecutionRequest
    const size_t exeRequestSerSize = RpcExecutionRequest().serialize().size();  // Worst method possible, must be changed, but it works for now...
    const std::vector<byte_t> serExeRequest = dataExchanger->receive(exeRequestSerSize);
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
    std::vector<byte_t> allParametersBuffer;
    size_t totalParametersSize = 0;
    for (size_t paramIndex = 0; paramIndex < exeRequest.paramCount; ++paramIndex)
        totalParametersSize += (size_t)exeRequest.params[paramIndex].size;
    if (totalParametersSize > 0) {
        allParametersBuffer = dataExchanger->receive(totalParametersSize);
        if (allParametersBuffer.size() != totalParametersSize) {
            return invalidRequestEventHandler(exeRequest, InvalidRequestReason::BAD_PARAMETERS);
        }
    }

    // Invoke function
    const uint64_t result = fd.rpcFunction(&exeRequest, allParametersBuffer);

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
