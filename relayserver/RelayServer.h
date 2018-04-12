#pragma once

#include <map>
#include <TcpServer/TcpServer.h>
#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>
#include "TcpProtocol.h"
#include "WebsocketConnection.h"

class RelayServer
{
	public:
		RelayServer();
		int Run();

	private:
		int _clientSocket;
		TcpServer _tcpServer;
		TcpProtocol _tcpProtocol;

		typedef websocketpp::server<websocketpp::config::core> WebsocketServer;
		WebsocketServer _websocketServer;
		std::map<int, WebsocketConnection> _connections;

		bool OnConnectionEstablished(TcpSocket &socket);
		bool OnConnectionClosed(TcpSocket &socket);
		bool OnDataAvailable(TcpSocket &socket);

		bool OnServerDataReceived(const epoll_event& ev);
};
