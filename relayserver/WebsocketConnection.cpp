#include "WebsocketConnection.h"
#include "TcpProtocol.h"
#include "MsgPackProtocol.h"

WebsocketConnection::WebsocketConnection(int socket, WebsocketServer::connection_ptr websocket)
	: _socket(socket), _websocket(websocket)
{
}

void WebsocketConnection::Eof()
{
	_websocket->eof();
}

void WebsocketConnection::DataReceived(const char *data, size_t count)
{
	_websocket->read_some(data, count);
}

void WebsocketConnection::FrameComplete(uint64_t frame_id, const TcpProtocol &proto)
{
	msgpack::sbuffer buf;
	msgpack::pack(buf, proto.GetGameInfo());
	_websocket->send(buf.data(), buf.size());
}
