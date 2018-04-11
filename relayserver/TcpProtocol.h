#pragma once
#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <deque>
#include <vector>
#include <memory>
#include "SpatialMap.h"

typedef std::uint64_t guid_t;
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

		struct Food
		{
			guid_t guid;
			Vector2D position;
			real_t value;

			const Vector2D& pos() const { return position; }
		};
		typedef SpatialMap<Food, SPATIAL_MAP_TILES_X, SPATIAL_MAP_TILES_Y> FoodMap;

		struct SnakeSegment
		{
			guid_t bot_id;
			Vector2D position;
			const Vector2D& pos() const { return position; }
		};
		typedef SpatialMap<SnakeSegment, SPATIAL_MAP_TILES_X, SPATIAL_MAP_TILES_Y> SnakeSegmentMap;

		struct Bot
		{
			guid_t guid;
			std::string name;
			real_t segment_radius;
			std::vector<SnakeSegment> segments;
			std::vector<uint32_t> color;
		};

		size_t _awaitedSize = 0;
		std::deque<char> _buf;
		MessageReceivedCallback _messageReceivedCallback;

		std::unique_ptr<FoodMap> _food;
		real_t _foodDecayRate = 0;

		std::vector<Bot> _bots;
		std::unique_ptr<SnakeSegmentMap> _segments;

		void OnMessageReceived(std::vector<char>& data);

		void OnWorldInfoReceived(real_t size_x, real_t size_y, real_t decay_rate);

		void OnFoodSpawnReceived(const Food& food);
		void OnFoodConsumedReceived(guid_t food_id, guid_t bot_id);
		void OnFoodDecayedReceived(guid_t food_id);

		void OnBotSpawnReceived(const Bot& bot);
		void OnBotKillReceived(guid_t killer_id, guid_t victim_id);
		void OnBotMoveReceived(guid_t bot_id, std::vector<TcpProtocol::SnakeSegment> &new_segments, size_t new_length, real_t new_segment_radius);
};
