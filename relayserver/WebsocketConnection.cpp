#include "WebsocketConnection.h"
#include "TcpProtocol.h"
#include "MsgPackProtocol.h"

WebsocketConnection::WebsocketConnection(uWS::WebSocket<uWS::SERVER> *websocket)
	: _websocket(websocket)
{
}

void WebsocketConnection::FrameComplete(uint64_t frame_id, const TcpProtocol &proto)
{
	(void) frame_id;
	if (!_firstFrameSent)
	{
		sendInitialData(proto);
		_firstFrameSent = true;
	}

	msgpack::sbuffer buf;
	for (auto& msg: proto.GetPendingMessages())
	{
		buf.clear();
		msg->pack(buf);
		_websocket->send(buf.data(), buf.size(), uWS::OpCode::BINARY);
	}
}

void WebsocketConnection::sendInitialData(const TcpProtocol &proto)
{
	msgpack::sbuffer buf;
	buf.clear();
	proto.GetGameInfo().pack(buf);
	_websocket->send(buf.data(), buf.size(), uWS::OpCode::BINARY);

	buf.clear();
	proto.GetWorldUpdate().pack(buf);
	_websocket->send(buf.data(), buf.size(), uWS::OpCode::BINARY);
}
