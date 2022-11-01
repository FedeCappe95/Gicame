#ifndef __GICAME__TCPLISTENINGSOCKET_H__
#define __GICAME__TCPLISTENINGSOCKET_H__


#include <string>
#include <vector>
#include "../common.h"
#include "../interfaces/IDataExchanger.h"
#include "./NetworkUtility.h"
#include "./NetworkDefinitions.h"
#include "./TcpSocket.h"


namespace Gicame {

	class GICAME_API TcpListeningSocket {

		MOVABLE_BUT_NOT_COPYABLE;

	private:
		static constexpr uint16_t DEFAULT_BACKLOG = 100u;

	private:
		SocketDescriptor sockfd;
		bool listening;
		short sin_family; //IPv4 (AF_INET) or IPv6 (AF_INET6)

	public:
		TcpListeningSocket(const InternetProtocolVersion ipv = InternetProtocolVersion::IPv4);
		~TcpListeningSocket();
		bool listenTo(const uint16_t port, const uint16_t backlog = DEFAULT_BACKLOG);
		TcpSocket acceptRequest();
		void close();

	};

};


#endif
