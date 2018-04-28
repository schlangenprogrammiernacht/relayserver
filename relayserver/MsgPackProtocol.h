#pragma once

#include <vector>
#include <msgpack.hpp>
#include "types.h"

namespace MsgPackProtocol
{
	typedef enum
	{
		MESSAGE_TYPE_GAME_INFO = 0x00,
		MESSAGE_TYPE_WORLD_UPDATE = 0x01,

		MESSAGE_TYPE_TICK = 0x10,

		MESSAGE_TYPE_BOT_SPAWN = 0x20,
		MESSAGE_TYPE_BOT_KILL = 0x21,
		MESSAGE_TYPE_BOT_MOVE = 0x22,

		MESSAGE_TYPE_FOOD_SPAWN = 0x30,
		MESSAGE_TYPE_FOOD_CONSUME = 0x31,
		MESSAGE_TYPE_FOOD_DECAY = 0x32,

		MESSAGE_TYPE_PLAYER_INFO = 0xF0,
	} MessageType;

	static constexpr const uint8_t PROTOCOL_VERSION = 1;

	struct FoodItem
	{
		guid_t guid;
		Vector2D position;
		real_t value;

		const Vector2D& pos() const { return position; }
	};

	struct SnakeSegmentItem
	{
		guid_t bot_id;
		Vector2D position;
		const Vector2D& pos() const { return position; }
	};

	struct BotItem
	{
		guid_t guid;
		std::string name;
		real_t segment_radius;
		std::vector<SnakeSegmentItem> segments;
		std::vector<uint32_t> color;
		int database_id;
	};

	struct Message
	{
		MessageType messageType;
		Message(MessageType type) : messageType(type) {}
		virtual ~Message() = default;
	};

	struct GameInfoMessage : public Message
	{
		double world_size_x = 0;
		double world_size_y = 0;
		double food_decay_per_frame = 0;
		GameInfoMessage(): Message(MESSAGE_TYPE_GAME_INFO) {}
	};

	struct PlayerInfoMessage : public Message
	{
		guid_t player_id; // id der von dieser Verbindung gesteuerten Schlange
		PlayerInfoMessage(): Message(MESSAGE_TYPE_PLAYER_INFO) {}
	};

	struct TickMessage : public Message
	{
		guid_t frame_id; // frame counter since start of server
		TickMessage(): Message(MESSAGE_TYPE_TICK) {}
	};

	struct WorldUpdateMessage : public Message
	{
		std::vector<BotItem> bots;
		std::vector<FoodItem> food;
		WorldUpdateMessage(): Message(MESSAGE_TYPE_WORLD_UPDATE) {}
	};

	struct BotSpawnMessage : public Message
	{
		BotItem bot;
		BotSpawnMessage(): Message(MESSAGE_TYPE_BOT_SPAWN) {}
	};

	struct BotMoveItem
	{
		guid_t bot_id;
		std::vector<SnakeSegmentItem> new_segments;
		size_t current_length;
		real_t current_segment_radius;
	};

	struct BotMoveMessage : public Message
	{
		std::vector<BotMoveItem> items;
		BotMoveMessage(): Message(MESSAGE_TYPE_BOT_MOVE) {}
	};

	struct BotKillMessage : public Message
	{
		guid_t killer_id;
		guid_t victim_id; // victim is deleted in this frame
		BotKillMessage(): Message(MESSAGE_TYPE_BOT_KILL) {}
	};

	struct FoodSpawnMessage : public Message
	{
		std::vector<FoodItem> new_food;
		FoodSpawnMessage(): Message(MESSAGE_TYPE_FOOD_SPAWN) {}
	};

	struct FoodConsumeItem
	{
		guid_t food_id; // food is deleted in this frame
		guid_t bot_id; // bot consuming the food
	};

	struct FoodConsumeMessage : public Message
	{
		std::vector<FoodConsumeItem> items;
		FoodConsumeMessage(): Message(MESSAGE_TYPE_FOOD_CONSUME) {}
	};

	struct FoodDecayMessage : public Message
	{
		std::vector<guid_t> food_ids; // food is deleted in this frame
		FoodDecayMessage(): Message(MESSAGE_TYPE_FOOD_DECAY) {}
	};

	void pack(msgpack::sbuffer& buf, const Message& msg);
}

namespace msgpack {
	MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
		namespace adaptor {

			template <> struct pack<MsgPackProtocol::GameInfoMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::GameInfoMessage const& v) const
				{
					o.pack_array(5);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_GAME_INFO));
					o.pack(v.world_size_x);
					o.pack(v.world_size_y);
					o.pack(v.food_decay_per_frame);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::GameInfoMessage>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::GameInfoMessage& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 5) throw msgpack::type_error();
					o.via.array.ptr[2] >> v.world_size_x;
					o.via.array.ptr[3] >> v.world_size_y;
					o.via.array.ptr[4] >> v.food_decay_per_frame;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::PlayerInfoMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::PlayerInfoMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_PLAYER_INFO));
					o.pack(v.player_id);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::PlayerInfoMessage>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::PlayerInfoMessage& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 3) throw msgpack::type_error();
					o.via.array.ptr[2] >> v.player_id;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::TickMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::TickMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_TICK));
					o.pack(v.frame_id);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::TickMessage>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::TickMessage& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 3) throw msgpack::type_error();
					o.via.array.ptr[2] >> v.frame_id;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::WorldUpdateMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::WorldUpdateMessage const& v) const
				{
					o.pack_array(4);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_WORLD_UPDATE));
					o.pack(v.bots);
					o.pack(v.food);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::WorldUpdateMessage>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::WorldUpdateMessage& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 4) throw msgpack::type_error();
					o.via.array.ptr[2] >> v.bots;
					o.via.array.ptr[3] >> v.food;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotSpawnMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotSpawnMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_BOT_SPAWN));
					o.pack(v.bot);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::BotSpawnMessage>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::BotSpawnMessage& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 3) throw msgpack::type_error();
					o.via.array.ptr[2] >> v.bot;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotMoveMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotMoveMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_BOT_MOVE));
					o.pack(v.items);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::BotMoveMessage>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::BotMoveMessage& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 3) throw msgpack::type_error();
					o.via.array.ptr[2] >> v.items;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotMoveItem>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotMoveItem const& v) const
				{
					o.pack_array(4);
					o.pack(v.bot_id);
					o.pack(v.new_segments);
					o.pack(v.current_length);
					o.pack(v.current_segment_radius);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::BotMoveItem>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::BotMoveItem& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 4) throw msgpack::type_error();
					o.via.array.ptr[0] >> v.bot_id;
					o.via.array.ptr[1] >> v.new_segments;
					o.via.array.ptr[2] >> v.current_length;
					o.via.array.ptr[3] >> v.current_segment_radius;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotKillMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotKillMessage const& v) const
				{
					o.pack_array(4);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_BOT_KILL));
					o.pack(v.killer_id);
					o.pack(v.victim_id);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::BotKillMessage>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::BotKillMessage& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 4) throw msgpack::type_error();
					o.via.array.ptr[2] >> v.killer_id;
					o.via.array.ptr[3] >> v.victim_id;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::FoodSpawnMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::FoodSpawnMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_FOOD_SPAWN));
					o.pack(v.new_food);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::FoodSpawnMessage>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::FoodSpawnMessage& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 3) throw msgpack::type_error();
					o.via.array.ptr[2] >> v.new_food;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::FoodConsumeMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::FoodConsumeMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_FOOD_CONSUME));
					o.pack(v.items);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::FoodConsumeMessage>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::FoodConsumeMessage& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 3) throw msgpack::type_error();
					o.via.array.ptr[2] >> v.items;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::FoodConsumeItem>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::FoodConsumeItem const& v) const
				{
					o.pack_array(2);
					o.pack(v.food_id);
					o.pack(v.bot_id);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::FoodConsumeItem>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::FoodConsumeItem& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 2) throw msgpack::type_error();
					o.via.array.ptr[0] >> v.food_id;
					o.via.array.ptr[1] >> v.bot_id;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::FoodDecayMessage>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::FoodDecayMessage const& v) const
				{
					o.pack_array(3);
					o.pack(MsgPackProtocol::PROTOCOL_VERSION);
					o.pack(static_cast<int>(MsgPackProtocol::MESSAGE_TYPE_FOOD_DECAY));
					o.pack(v.food_ids);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::FoodDecayMessage>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::FoodDecayMessage& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 3) throw msgpack::type_error();
					o.via.array.ptr[2] >> v.food_ids;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::SnakeSegmentItem>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::SnakeSegmentItem const& v) const
				{
					o.pack_array(2);
					o.pack(v.pos().x());
					o.pack(v.pos().y());
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::SnakeSegmentItem>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::SnakeSegmentItem& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 2) throw msgpack::type_error();
					v.bot_id = 0;
					v.position = {
						o.via.array.ptr[0].via.f64,
						o.via.array.ptr[1].via.f64,
					};
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::FoodItem>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::FoodItem const& v) const
				{
					o.pack_array(4);
					o.pack(v.guid);
					o.pack(v.pos().x());
					o.pack(v.pos().y());
					o.pack(v.value);
					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::FoodItem>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::FoodItem& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 4) throw msgpack::type_error();
					o.via.array.ptr[0] >> v.guid;
					v.position = {
						o.via.array.ptr[1].via.f64,
						o.via.array.ptr[2].via.f64,
					};
					o.via.array.ptr[3] >> v.value;
					return o;
				}
			};

			template <> struct pack<MsgPackProtocol::BotItem>
			{
				template <typename Stream> msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, MsgPackProtocol::BotItem const& v) const
				{
					o.pack_array(5);
					o.pack(v.guid);
					o.pack(v.name);
					o.pack(v.segment_radius);

					// segments
					o.pack(v.segments);

					// FIXME: colormap: array of RGB values
					o.pack_array(3);
					o.pack(0xFF0000);
					o.pack(0x00FF00);
					o.pack(0x0000FF);

					return o;
				}
			};

			template<> struct convert<MsgPackProtocol::BotItem>
			{
				msgpack::object const& operator()(msgpack::object const& o, MsgPackProtocol::BotItem& v) const
				{
					if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
					if (o.via.array.size != 6) throw msgpack::type_error();
					o.via.array.ptr[0] >> v.guid;
					o.via.array.ptr[1] >> v.name;
					o.via.array.ptr[2] >> v.segment_radius;
					o.via.array.ptr[3] >> v.segments;
					o.via.array.ptr[4] >> v.color;
					o.via.array.ptr[5] >> v.database_id;
					for (auto &segmentItem: v.segments)
					{
						segmentItem.bot_id = v.guid;
					}
					return o;
				}
			};

		} // namespace adaptor
	} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
