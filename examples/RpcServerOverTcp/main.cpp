#include <iostream>
#include <stdint.h>
#include "../../headers/network/TcpSocket.h"
#include "../../headers/network/TcpListeningSocket.h"
#include "../../headers/rpc/RpcServer.h"
#include "./RemoteFunctions.h"


using namespace Gicame;


static void sayHello() {
	std::cout << "Hello!!" << std::endl;
}


static uint64_t sum(const uint32_t a, const uint32_t b) {
	std::cout << "function sum(...) invoked. a = " << a << ", b = " << b << std::endl;
	return a + b;
}


int main() {
	std::cout << "Gicame example: RpcServerOverTcp" << std::endl;

	TcpListeningSocket serverSocket(InternetProtocolVersion::IPv4);
	serverSocket.listenTo(9898);
	TcpSocket socket = serverSocket.acceptRequest();

	RpcServer rpcServer(&socket);
	rpcServer.registerFunction(sayHello, RPC_ID_SAY_HELLO);
	std::function<uint64_t(uint32_t, uint32_t)> s = sum;
	rpcServer.registerFunction(s, RPC_ID_SUM);
	rpcServer.run();


	return 0;
}