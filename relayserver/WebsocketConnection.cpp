#include "WebsocketConnection.h"
#include "TcpProtocol.h"
#include "MsgPackProtocol.h"

WebsocketConnection::WebsocketConnection(int socket, WebsocketServer::connection_ptr websocket)
	: _socket(socket), _websocket(websocket)
{
}

void WebsocketConnection::Eof()
{
	_websocket->fatal_error();
}

void WebsocketConnection::DataReceived(const char *data, size_t count)
{
	_websocket->read_all(data, count);
}

void WebsocketConnection::FrameComplete(uint64_t frame_id, const TcpProtocol &proto)
{
	if (!_firstFrameSent)
	{
		msgpack::sbuffer buf;
		msgpack::pack(buf, proto.GetGameInfo());
		_websocket->send(buf.data(), buf.size());

		buf.clear();
		proto.GetWorldUpdate(buf);
		_websocket->send(buf.data(), buf.size());

		_firstFrameSent = true;
	}
	else
	{
		msgpack::sbuffer buf;
		proto.GetFoodSpawnMessages(buf);
		_websocket->send(buf.data(), buf.size());
		buf.clear();
		proto.GetFoodConsumeMessages(buf);
		_websocket->send(buf.data(), buf.size());
		buf.clear();
		proto.GetFoodDecayMessages(buf);
		_websocket->send(buf.data(), buf.size());
	}
}
