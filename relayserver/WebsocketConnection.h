#pragma once

#include <uWS.h>

class TcpProtocol;

class WebsocketConnection
{
	public:
		WebsocketConnection(uWS::WebSocket<uWS::SERVER> *websocket);
		void FrameComplete(uint64_t frame_id, const TcpProtocol& proto);
		void sendString(std::string data);

	private:
		uWS::WebSocket<uWS::SERVER> *_websocket;
		bool _firstFrameSent = false;

		void sendInitialData(const TcpProtocol& proto);

};
