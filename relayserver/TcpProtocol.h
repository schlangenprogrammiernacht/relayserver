#pragma once
#include <stddef.h>
#include <stdint.h>
#include <functional>
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
		typedef std::function<void(uint64_t frame_id)> FrameCompleteCallback;
		static constexpr const size_t BUFFER_SIZE = 1024*1024;

		TcpProtocol();
		void SetFrameCompleteCallback(FrameCompleteCallback callback);
		bool Read(int socket);

	private:
		static constexpr const size_t SPATIAL_MAP_TILES_X = 128;
		static constexpr const size_t SPATIAL_MAP_TILES_Y = 128;
		static constexpr const size_t SPATIAL_MAP_RESERVE_COUNT = 10;

		typedef SpatialMap<FoodItem, SPATIAL_MAP_TILES_X, SPATIAL_MAP_TILES_Y> FoodMap;

		typedef SpatialMap<SnakeSegmentItem, SPATIAL_MAP_TILES_X, SPATIAL_MAP_TILES_Y> SnakeSegmentMap;

		std::vector<char> _buf;
		size_t _bufHead=0;
		size_t _bufTail=0;

		FrameCompleteCallback _frameCompleteCallback;
		std::unique_ptr<FoodMap> _food;
		MsgPackProtocol::GameInfoMessage _gameInfo;

		std::vector<BotItem> _bots;
		std::unique_ptr<SnakeSegmentMap> _segments;

		void OnMessageReceived(const char *data, size_t count);

		void OnGameInfoReceived(const MsgPackProtocol::GameInfoMessage& msg);
		void OnWorldUpdateReceived(const MsgPackProtocol::WorldUpdateMessage& msg);
		void OnTickReceived(const MsgPackProtocol::TickMessage& msg);

		void OnFoodSpawnReceived(const MsgPackProtocol::FoodSpawnMessage& msg);
		void OnFoodConsumedReceived(const MsgPackProtocol::FoodConsumeMessage& msg);
		void OnFoodDecayedReceived(const MsgPackProtocol::FoodDecayMessage& msg);

		void OnBotSpawnReceived(const MsgPackProtocol::BotSpawnMessage& msg);
		void OnBotKillReceived(const MsgPackProtocol::BotKillMessage &msg);
		void OnBotMoveReceived(const MsgPackProtocol::BotMoveMessage &msg);
};
