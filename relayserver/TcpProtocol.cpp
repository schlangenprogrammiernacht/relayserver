#include "TcpProtocol.h"
#include <stdint.h>
#include <unistd.h>
#include <array>
#include <msgpack.hpp>
#include <iostream>

void TcpProtocol::SetMessageReceivedCallback(TcpProtocol::MessageReceivedCallback callback)
{
	_messageReceivedCallback = callback;
}

bool TcpProtocol::Read(int socket)
{
	std::array<char, 1024> readbuf;
	ssize_t bytesRead = read(socket, readbuf.data(), readbuf.size());
	if (bytesRead<=0) { return false; }
	_buf.insert(_buf.end(), &readbuf[0], &readbuf[static_cast<size_t>(bytesRead)]);

	while (true)
	{
		if ((_awaitedSize==0) && (_buf.size() >= 4))
		{
			_awaitedSize = static_cast<uint8_t>(_buf[0])<<24
						 | static_cast<uint8_t>(_buf[1])<<16
						 | static_cast<uint8_t>(_buf[2])<<8
						 | static_cast<uint8_t>(_buf[3])<<0;
			_buf.erase(_buf.begin(), _buf.begin()+4);
		}
		else if ((_awaitedSize>0) && (_buf.size() >= _awaitedSize))
		{
			std::vector<char> v(_buf.begin(), _buf.begin()+_awaitedSize);
			OnMessageReceived(v);
			_buf.erase(_buf.begin(), _buf.begin()+_awaitedSize);
			_awaitedSize = 0;
		}
		else
		{
			return true;
		}
	}
}

void TcpProtocol::OnMessageReceived(std::vector<char> &data)
{
	_messageReceivedCallback(data);

	msgpack::unpacker pac;
	msgpack::object_handle obj;

	pac.reserve_buffer(data.size());
	memcpy(pac.buffer(), data.data(), data.size());
	pac.buffer_consumed(data.size());

	if (!pac.next(obj)) { return; }
	if (obj.get().type != msgpack::type::ARRAY) { return; }
	auto arr = obj.get().via.array;
	if (arr.size<2) { return; }

	uint64_t version = arr.ptr[0].via.u64;
	uint64_t message_type = arr.ptr[1].via.u64;

	std::cerr << "version: " << version << " message type: " << message_type << std::endl;
	switch (message_type)
	{
		case 0x00:
			if (arr.size<5) { return; }
			std::cerr << "size_x " << arr.ptr[2].via.f64 << " size_y " << arr.ptr[3].via.f64 << " decrate " << arr.ptr[4].via.f64 << std::endl;
			break;
		case 0x01:
			std::cerr << "world update" << std::endl;
			break;
		case 0x10:
			std::cerr << "tick" << std::endl;
			break;
		case 0x20:
			std::cerr << "bot spawn" << std::endl;
			break;
		case 0x21:
			std::cerr << "bot kill" << std::endl;
			break;
		case 0x22:
			std::cerr << "bot move" << std::endl;
			break;
		case 0x30:
			std::cerr << "food spawn" << std::endl;
			break;
		case 0x31:
			std::cerr << "food consume" << std::endl;
			break;
		case 0x32:
			std::cerr << "food decay" << std::endl;
			break;
	}

}
