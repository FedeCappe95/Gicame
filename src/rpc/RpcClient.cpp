#include "../headers/rpc/RpcClient.h"


using namespace Gicame;


RpcClient::RpcClient(IDataExchanger* dataExchanger, const BinarySerializer& serializer) :
    dataExchanger(dataExchanger),
    serializer(serializer)
{
    // Nothing here
}

std::vector<byte_t> RpcClient::sendExeRequest(const RpcExecutionRequest& rer, const std::vector<byte_t> paramsDump, const size_t replySize) {
    // Data exchange
    std::vector<byte_t> buffer(sizeof(rer) + paramsDump.size());
    rer.serialize(buffer.data());
    std::copy(paramsDump.data(), paramsDump.data() + paramsDump.size(), buffer.data() + sizeof(rer));
    dataExchanger->send(buffer);

    // Receive result
    const std::vector<byte_t> reply = dataExchanger->receive(replySize);
    return reply;
}
