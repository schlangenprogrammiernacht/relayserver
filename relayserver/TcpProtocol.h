#pragma once
#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <deque>
#include <vector>
#include <memory>
#include "SpatialMap.h"
#include "MsgPackProtocol.h"

using BotItem = MsgPackProtocol::BotItem;
using FoodItem = MsgPackProtocol::FoodItem;
using SnakeSegmentItem = MsgPackProtocol::SnakeSegmentItem;

class TcpProtocol
{
	public:
		typedef std::function<void(std::vector<char>&)> MessageReceivedCallback;
		void SetMessageReceivedCallback(MessageReceivedCallback callback);
		bool Read(int socket);

	private:
		static constexpr const size_t SPATIAL_MAP_TILES_X = 128;
		static constexpr const size_t SPATIAL_MAP_TILES_Y = 128;
		static constexpr const size_t SPATIAL_MAP_RESERVE_COUNT = 10;

		typedef SpatialMap<FoodItem, SPATIAL_MAP_TILES_X, SPATIAL_MAP_TILES_Y> FoodMap;

		typedef SpatialMap<SnakeSegmentItem, SPATIAL_MAP_TILES_X, SPATIAL_MAP_TILES_Y> SnakeSegmentMap;


		size_t _awaitedSize = 0;
		std::deque<char> _buf;
		MessageReceivedCallback _messageReceivedCallback;

		std::unique_ptr<FoodMap> _food;
		MsgPackProtocol::GameInfoMessage _gameInfo;

		std::vector<BotItem> _bots;
		std::unique_ptr<SnakeSegmentMap> _segments;

		void OnMessageReceived(std::vector<char>& data);

		void OnGameInfoReceived(const MsgPackProtocol::GameInfoMessage& msg);

		void OnFoodSpawnReceived(const FoodItem& food);
		void OnFoodConsumedReceived(const MsgPackProtocol::FoodConsumeItem& item);
		void OnFoodDecayedReceived(guid_t food_id);

		void OnBotSpawnReceived(const MsgPackProtocol::BotItem& bot);
		void OnBotKillReceived(const MsgPackProtocol::BotKillMessage &msg);
		void OnBotMoveReceived(const MsgPackProtocol::BotMoveItem &item);
};
