#pragma once

#include <map>
#include <TcpServer/TcpServer.h>
#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>
#include "TcpProtocol.h"

class RelayServer
{
	public:
		RelayServer();
		int Run();

	private:
		int _clientSocket;
		TcpServer _tcpServer;
		TcpProtocol msg;

		typedef websocketpp::server<websocketpp::config::core> WebsocketServer;

		WebsocketServer _websocketServer;
		std::map<int, WebsocketServer::connection_ptr> _websocketConnections;

		bool OnConnectionEstablished(TcpSocket &socket);
		bool OnConnectionClosed(TcpSocket &socket);
		bool OnDataAvailable(TcpSocket &socket);

		bool OnServerDataReceived(const epoll_event& ev);
};
