#include "WebsocketConnection.h"
#include "TcpProtocol.h"
#include "MsgPackProtocol.h"

WebsocketConnection::WebsocketConnection(uWS::WebSocket<uWS::SERVER> *websocket)
	: _websocket(websocket)
{
}

void WebsocketConnection::FrameComplete(uint64_t frame_id, const TcpProtocol &proto)
{
	if (!_firstFrameSent)
	{
		msgpack::sbuffer buf;
		msgpack::pack(buf, proto.GetGameInfo());
		_websocket->send(buf.data(), buf.size(), uWS::OpCode::BINARY);

		buf.clear();
		proto.GetWorldUpdate(buf);
		_websocket->send(buf.data(), buf.size(), uWS::OpCode::BINARY);

		_firstFrameSent = true;
	}
	else
	{
		msgpack::sbuffer buf;
		proto.GetFoodSpawnMessages(buf);
		_websocket->send(buf.data(), buf.size(), uWS::OpCode::BINARY);
		buf.clear();
		proto.GetFoodConsumeMessages(buf);
		_websocket->send(buf.data(), buf.size(), uWS::OpCode::BINARY);
		buf.clear();
		proto.GetFoodDecayMessages(buf);
		_websocket->send(buf.data(), buf.size(), uWS::OpCode::BINARY);
	}
}
