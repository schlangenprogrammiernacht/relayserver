#pragma once

#include <uWS.h>
#include "TcpProtocol.h"
#include "WebsocketConnection.h"

class RelayServer
{
	public:
		RelayServer();
		int Run();
	private:
		int _clientSocket;
		TcpProtocol _tcpProtocol;
};
