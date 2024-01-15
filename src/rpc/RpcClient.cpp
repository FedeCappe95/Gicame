#include "../headers/rpc/RpcClient.h"


using namespace Gicame;


RpcClient::RpcClient(IDataExchanger* dataExchanger, const BinarySerializer& serializer) :
    dataExchanger(dataExchanger),
    serializer(serializer)
{
    // Nothing here
}

uint64_t RpcClient::sendExeRequest(const RpcExecutionRequest& rer, const std::vector<byte_t> paramsDump) {
    // Data exchange
    std::vector<byte_t> buffer(sizeof(rer) + paramsDump.size());
    rer.serialize(buffer.data());
    std::copy(paramsDump.data(), paramsDump.data() + paramsDump.size(), buffer.data() + sizeof(rer));
    dataExchanger->send(buffer);

    // Receive result
    const std::vector<byte_t> reply = dataExchanger->receive(sizeof(uint64_t));
    return *((uint64_t*)reply.data());
}
