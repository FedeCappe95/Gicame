#include <iostream>
#include <stdint.h>
#include "../../headers/network/TcpSocket.h"
#include "../../headers/rpc/RpcClient.h"
#include "../RpcServerOverTcp/RemoteFunctions.h"


using namespace Gicame;


int main() {
	std::cout << "Gicame example: RpcClientOverTcp" << std::endl;

	// Connect to the server
	TcpSocket socket(InternetProtocolVersion::IPv4);
	const bool connectedToServer = socket.connectTo(IPv4("127.0.0.1"), 9898);       // Connect to localhost on port 9898
	if (!connectedToServer) {
		std::cerr << "Unable to connect to the server" << std::endl;
		return -1;
	}

	// Create the RpcClient and retrieve handles for the remote procedures
	RpcClient rpcClient(&socket);
	auto sayHello = rpcClient.get<uint64_t>(RPC_ID_SAY_HELLO);
	auto sum = rpcClient.get<uint32_t, uint32_t, uint32_t>(RPC_ID_SUM);

	// Invoke the remote procedures
	sayHello();
	sum(4, 5);

	// Close
	socket.close();

	return 0;
}