#pragma once
#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <vector>
#include <memory>
#include <map>
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

		const MsgPackProtocol::GameInfoMessage& GetGameInfo() const { return _gameInfo; }
		const MsgPackProtocol::WorldUpdateMessage& GetWorldUpdate() const { return _worldUpdate; }

		const std::vector<std::unique_ptr<MsgPackProtocol::Message>>& GetPendingMessages() const { return _pendingMessages; }
		typedef std::map<uint64_t, std::vector<MsgPackProtocol::BotLogItem>> LogItemMap;
		const LogItemMap& GetPendingLogItems() const { return _pendingLogItems; }
		void ClearLogItems();

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
		MsgPackProtocol::GameInfoMessage _gameInfo;
		MsgPackProtocol::WorldUpdateMessage _worldUpdate;
		std::vector<FoodItem>& _food;
		std::vector<BotItem>& _bots;
		std::vector<std::unique_ptr<MsgPackProtocol::Message>> _pendingMessages;

		LogItemMap _pendingLogItems;

		void OnMessageReceived(const char *data, size_t count);

		void OnGameInfoReceived(const MsgPackProtocol::GameInfoMessage& msg);
		void OnWorldUpdateReceived(const MsgPackProtocol::WorldUpdateMessage& msg);
		void OnTickReceived(const MsgPackProtocol::TickMessage& msg);

		void OnFoodSpawnReceived(const MsgPackProtocol::FoodSpawnMessage& msg);
		void OnFoodConsumedReceived(const MsgPackProtocol::FoodConsumeMessage& msg);
		void OnFoodDecayedReceived(const MsgPackProtocol::FoodDecayMessage& msg);

		void OnBotSpawnReceived(const MsgPackProtocol::BotSpawnMessage& msg);
		void OnBotKillReceived(const MsgPackProtocol::BotKillMessage &msg);
		void OnBotMoveReceived(std::unique_ptr<MsgPackProtocol::BotMoveMessage> msg);
		void OnBotLogReceived(std::unique_ptr<MsgPackProtocol::BotLogMessage> msg);
};
