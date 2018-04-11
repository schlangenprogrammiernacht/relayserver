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
		case MsgPackProtocol::MESSAGE_TYPE_GAME_INFO:
			OnGameInfoReceived(obj.get().as<MsgPackProtocol::GameInfoMessage>());
			break;

		case MsgPackProtocol::MESSAGE_TYPE_WORLD_UPDATE:
		{
			auto msg = obj.get().as<MsgPackProtocol::WorldUpdateMessage>();
			for (auto& bot: msg.bots)
			{
				OnBotSpawnReceived(bot);
			}
			for (auto& food: msg.food)
			{
				OnFoodSpawnReceived(food);
			}
			break;
		}

		case MsgPackProtocol::MESSAGE_TYPE_TICK:
			std::cerr << "tick" << std::endl;
			break;

		case MsgPackProtocol::MESSAGE_TYPE_BOT_SPAWN:
		{
			auto msg = obj.get().as<MsgPackProtocol::BotSpawnMessage>();
			OnBotSpawnReceived(msg.bot);
			break;
		}

		case MsgPackProtocol::MESSAGE_TYPE_BOT_KILL:
			OnBotKillReceived(obj.get().as<MsgPackProtocol::BotKillMessage>());
			break;

		case MsgPackProtocol::MESSAGE_TYPE_BOT_MOVE:
		{
			auto msg = obj.get().as<MsgPackProtocol::BotMoveMessage>();
			for (auto& item: msg.items)
			{
				OnBotMoveReceived(item);
			}
			break;
		}

		case MsgPackProtocol::MESSAGE_TYPE_FOOD_SPAWN:
		{
			auto msg = obj.get().as<MsgPackProtocol::FoodSpawnMessage>();
			for (auto& item: msg.new_food)
			{
				OnFoodSpawnReceived(item);
			}
			break;
		}

		case MsgPackProtocol::MESSAGE_TYPE_FOOD_CONSUME:
		{
			auto msg = obj.get().as<MsgPackProtocol::FoodConsumeMessage>();
			for (auto& item: msg.items)
			{
				OnFoodConsumedReceived(item);
			}
			break;
		}

		case MsgPackProtocol::MESSAGE_TYPE_FOOD_DECAY:
			auto msg = obj.get().as<MsgPackProtocol::FoodDecayMessage>();
			for (auto food_id: msg.food_ids)
			{
				OnFoodDecayedReceived(food_id);
			}
			break;
	}
}

void TcpProtocol::OnGameInfoReceived(const MsgPackProtocol::GameInfoMessage& msg)
{
	_segments = std::make_unique<SnakeSegmentMap>(msg.world_size_x, msg.world_size_y, 1000);
	_food = std::make_unique<FoodMap>(msg.world_size_x, msg.world_size_y, 1000);
	_gameInfo = msg;
}

void TcpProtocol::OnFoodSpawnReceived(const FoodItem  &food)
{
	if (_food == nullptr) { return; }
	_food->addElement(food);
}

void TcpProtocol::OnFoodConsumedReceived(const MsgPackProtocol::FoodConsumeItem &item)
{
	if (_food == nullptr) { return; }
	_food->erase_if([item](const FoodItem& food) { return food.guid == item.food_id; });
}

void TcpProtocol::OnFoodDecayedReceived(guid_t food_id)
{
	if (_food == nullptr) { return; }
	_food->erase_if([food_id](const FoodItem& food) { return food.guid == food_id; });
}

void TcpProtocol::OnBotSpawnReceived(const MsgPackProtocol::BotItem &bot)
{
	_bots.push_back(bot);
}

void TcpProtocol::OnBotKillReceived(const MsgPackProtocol::BotKillMessage& msg)
{
	_bots.erase(std::remove_if(_bots.begin(), _bots.end(), [msg](const BotItem& bot) { return bot.guid == msg.victim_id; }));
}

void TcpProtocol::OnBotMoveReceived(const MsgPackProtocol::BotMoveItem &item)
{
	auto it = std::find_if(_bots.begin(), _bots.end(), [item](const BotItem& bot) { return bot.guid == item.bot_id; });
	if (it == _bots.end()) { return; }
	auto& bot = *it;

	bot.segments.insert(bot.segments.begin(), item.new_segments.begin(), item.new_segments.end());
	bot.segments.resize(item.current_length);
	bot.segment_radius = item.current_segment_radius;
}

