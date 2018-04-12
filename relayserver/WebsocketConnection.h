#pragma once

#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>

class TcpProtocol;

class WebsocketConnection
{
	public:
		typedef websocketpp::server<websocketpp::config::core> WebsocketServer;

		WebsocketConnection(int socket, WebsocketServer::connection_ptr websocket);
		void Eof();
		void DataReceived(const char *data, size_t count);
		void FrameComplete(uint64_t frame_id, const TcpProtocol& proto);

	private:
		int _socket;
		WebsocketServer::connection_ptr _websocket;
		bool _firstFrameSent = false;

};
