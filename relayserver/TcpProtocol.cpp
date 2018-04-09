#include "TcpProtocol.h"
#include <stdint.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <array>

void TcpProtocol::SetMessageReceivedCallback(TcpProtocol::MessageReceivedCallback callback)
{
	_messageReceivedCallback = callback;
}

bool TcpProtocol::Read(int socket)
{
	std::array<uint8_t, 1024> readbuf;
	ssize_t bytesRead = read(socket, readbuf.data(), readbuf.size());
	if (bytesRead<=0) { return false; }
	_buf.insert(_buf.end(), &readbuf[0], &readbuf[static_cast<size_t>(bytesRead)]);

	if (_awaitedSize==0)
	{
		if (_buf.size()<4) { return true; }
		_awaitedSize = (_buf[0]<<24 | _buf[1]<<16 | _buf[2]<<8 | _buf[3]);
		_buf.erase(_buf.begin(), _buf.begin()+4);
	}

	if (_buf.size() >= _awaitedSize)
	{
		_messageReceivedCallback(std::vector<uint8_t>(_buf.begin(), _buf.begin()+_awaitedSize));
		_buf.erase(_buf.begin(), _buf.begin()+_awaitedSize);
		_awaitedSize = 0;
	}

	return true;
}
