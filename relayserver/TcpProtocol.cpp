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
			OnWorldInfoReceived(arr.ptr[2].via.f64, arr.ptr[3].via.f64, arr.ptr[4].via.f64);
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
			if (arr.size<3) { return; }
			for (auto& item: arr.ptr[2].via.array)
			{
				auto &fi = item.via.array;
				if (fi.size < 3) { continue; }
				OnFoodSpawnReceived({
					static_cast<guid_t>(fi.ptr[0].via.i64), // guid
					{fi.ptr[1].via.f64, fi.ptr[2].via.f64}, // position
					fi.ptr[3].via.f64 // value
				});
			}
			break;

		case 0x31:
			if (arr.size<3) { return; }
			for (auto& item: arr.ptr[2].via.array)
			{
				auto &fi = item.via.array;
				if (fi.size < 2) { continue; }
				OnFoodConsumedReceived(
					static_cast<guid_t>(fi.ptr[0].via.i64), // food guid
					static_cast<guid_t>(fi.ptr[1].via.i64) // bot guid
				);
			}
			break;

		case 0x32:
			if (arr.size<3) { return; }
			for (auto& item: arr.ptr[2].via.array)
			{
				OnFoodDecayedReceived(static_cast<guid_t>(item.via.u64));
			}
			break;
	}

}

void TcpProtocol::OnWorldInfoReceived(real_t size_x, real_t size_y, real_t decay_rate)
{
	_food = std::make_unique<FoodMap>(size_x, size_y, 1000);
	_foodDecayRate = decay_rate;
}

void TcpProtocol::OnFoodSpawnReceived(const TcpProtocol::Food &food)
{
	if (_food == nullptr) { return; }
	_food->addElement(food);
}

void TcpProtocol::OnFoodConsumedReceived(guid_t food_id, guid_t bot_id)
{
	_food->erase_if([food_id](const Food& food) { return food.guid == food_id; });
}

void TcpProtocol::OnFoodDecayedReceived(guid_t food_id)
{
	_food->erase_if([food_id](const Food& food) { return food.guid == food_id; });
}
