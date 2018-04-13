#include "TcpProtocol.h"
#include <stdint.h>
#include <unistd.h>
#include <array>
#include <algorithm>
#include <msgpack.hpp>
#include <iostream>

TcpProtocol::TcpProtocol()
{
	_buf.resize(BUFFER_SIZE);
}

void TcpProtocol::SetFrameCompleteCallback(TcpProtocol::FrameCompleteCallback callback)
{
	_frameCompleteCallback = callback;
}

bool TcpProtocol::Read(int socket)
{
	ssize_t bytesRead = read(socket, &_buf[_bufTail], _buf.size()-_bufTail);

	if (bytesRead<=0) { return false; }
	_bufTail += static_cast<size_t>(bytesRead);

	while ((_bufTail - _bufHead)>=4)
	{
		size_t size = 4 + ntohl(*(reinterpret_cast<uint32_t*>(&_buf[_bufHead])));
		if (size > _buf.size()) { return false; }
		if (size <= (_bufTail - _bufHead))
		{
			OnMessageReceived(&_buf[_bufHead+4], size-4);
			_bufHead += size;
		}
		else
		{
			break;
		}
	}

	std::copy(_buf.begin()+_bufHead, _buf.begin()+_bufTail, _buf.begin());
	_bufTail -= _bufHead;
	_bufHead = 0;
	return true;
}

const MsgPackProtocol::GameInfoMessage &TcpProtocol::GetGameInfo() const
{
	return _gameInfo;
}

bool TcpProtocol::GetWorldUpdate(msgpack::sbuffer &buf) const
{
	MsgPackProtocol::WorldUpdateMessage msg;
	msg.bots = _bots;
	msg.food = _foodVect;
	msgpack::pack(buf, msg);
	return true;
}

bool TcpProtocol::GetFoodSpawnMessages(msgpack::sbuffer &buf) const
{
	for (auto& msg: _foodSpawnMessages)
	{
		msgpack::pack(buf, msg);
		// FIXME this will fail with multiple foodSpawnMessages, since only one message should be in one buffer
	}
	return true;
}

bool TcpProtocol::GetFoodConsumeMessages(msgpack::sbuffer &buf) const
{
	for (auto& msg: _foodConsumeMessages)
	{
		msgpack::pack(buf, msg);
		// FIXME this will fail with multiple foodSpawnMessages, since only one message should be in one buffer
	}
	return true;
}

bool TcpProtocol::GetFoodDecayMessages(msgpack::sbuffer &buf) const
{
	for (auto& msg: _foodDecayMessages)
	{
		msgpack::pack(buf, msg);
		// FIXME this will fail with multiple foodSpawnMessages, since only one message should be in one buffer
	}
	return true;
}

void TcpProtocol::OnMessageReceived(const char* data, size_t count)
{
	msgpack::object_handle obj;
	uint64_t version, message_type;

	msgpack::unpack(obj, data, count);
	if (obj.get().type != msgpack::type::ARRAY) { return; }

	auto arr = obj.get().via.array;
	if (arr.size<2) { return; }
	arr.ptr[0] >> version;
	arr.ptr[1] >> message_type;

	switch (message_type)
	{
		case MsgPackProtocol::MESSAGE_TYPE_GAME_INFO:
			OnGameInfoReceived(obj.get().as<MsgPackProtocol::GameInfoMessage>());
			break;

		case MsgPackProtocol::MESSAGE_TYPE_WORLD_UPDATE:
			OnWorldUpdateReceived(obj.get().as<MsgPackProtocol::WorldUpdateMessage>());
			break;

		case MsgPackProtocol::MESSAGE_TYPE_TICK:
			OnTickReceived(obj.get().as<MsgPackProtocol::TickMessage>());
			break;

		case MsgPackProtocol::MESSAGE_TYPE_BOT_SPAWN:
			OnBotSpawnReceived(obj.get().as<MsgPackProtocol::BotSpawnMessage>());
			break;

		case MsgPackProtocol::MESSAGE_TYPE_BOT_KILL:
			OnBotKillReceived(obj.get().as<MsgPackProtocol::BotKillMessage>());
			break;

		case MsgPackProtocol::MESSAGE_TYPE_BOT_MOVE:
			OnBotMoveReceived(obj.get().as<MsgPackProtocol::BotMoveMessage>());
			break;

		case MsgPackProtocol::MESSAGE_TYPE_FOOD_SPAWN:
			OnFoodSpawnReceived(obj.get().as<MsgPackProtocol::FoodSpawnMessage>());
			break;

		case MsgPackProtocol::MESSAGE_TYPE_FOOD_CONSUME:
			OnFoodConsumedReceived(obj.get().as<MsgPackProtocol::FoodConsumeMessage>());
			break;

		case MsgPackProtocol::MESSAGE_TYPE_FOOD_DECAY:
			OnFoodDecayedReceived(obj.get().as<MsgPackProtocol::FoodDecayMessage>());
			break;
	}
}

void TcpProtocol::OnGameInfoReceived(const MsgPackProtocol::GameInfoMessage& msg)
{
	_segments = std::make_unique<SnakeSegmentMap>(msg.world_size_x, msg.world_size_y, 1000);
	_foodMap = std::make_unique<FoodMap>(msg.world_size_x, msg.world_size_y, 1000);
	_gameInfo = msg;
}

void TcpProtocol::OnWorldUpdateReceived(const MsgPackProtocol::WorldUpdateMessage &msg)
{
	_bots = msg.bots;
	_foodVect = msg.food;
}

void TcpProtocol::OnTickReceived(const MsgPackProtocol::TickMessage &msg)
{
	_frameCompleteCallback(msg.frame_id);
	_foodSpawnMessages.clear();
	_foodConsumeMessages.clear();
	_foodDecayMessages.clear();
}

void TcpProtocol::OnFoodSpawnReceived(const MsgPackProtocol::FoodSpawnMessage& msg)
{
	_foodSpawnMessages.push_back(msg);
	_foodVect.insert(_foodVect.end(), msg.new_food.begin(), msg.new_food.end());
}

void TcpProtocol::OnFoodConsumedReceived(const MsgPackProtocol::FoodConsumeMessage &msg)
{
	_foodConsumeMessages.push_back(msg);
	_foodVect.erase(std::remove_if(_foodVect.begin(), _foodVect.end(), [&msg](const FoodItem& food) {
		for (auto& item: msg.items)
		{
			if (food.guid == item.food_id)
			{
				return true;
			}
		}
		return false;
	}));
}

void TcpProtocol::OnFoodDecayedReceived(const MsgPackProtocol::FoodDecayMessage &msg)
{
	_foodDecayMessages.push_back(msg);
	_foodVect.erase(std::remove_if(_foodVect.begin(), _foodVect.end(), [&msg](const FoodItem& food) {
		return std::find(msg.food_ids.begin(), msg.food_ids.end(), food.guid) != msg.food_ids.end();
	}));
}

void TcpProtocol::OnBotSpawnReceived(const MsgPackProtocol::BotSpawnMessage &msg)
{
	_bots.push_back(msg.bot);
}

void TcpProtocol::OnBotKillReceived(const MsgPackProtocol::BotKillMessage& msg)
{
	_bots.erase(std::remove_if(_bots.begin(), _bots.end(), [msg](const BotItem& bot) { return bot.guid == msg.victim_id; }));
}

void TcpProtocol::OnBotMoveReceived(const MsgPackProtocol::BotMoveMessage &msg)
{
	for (auto& item: msg.items)
	{
		auto it = std::find_if(_bots.begin(), _bots.end(), [item](const BotItem& bot) { return bot.guid == item.bot_id; });
		if (it == _bots.end()) { return; }
		auto& bot = *it;

		bot.segments.insert(bot.segments.begin(), item.new_segments.begin(), item.new_segments.end());
		bot.segments.resize(item.current_length);
		bot.segment_radius = item.current_segment_radius;
	}
}

