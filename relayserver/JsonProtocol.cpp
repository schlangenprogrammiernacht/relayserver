#include "JsonProtocol.h"

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::Message &msg)
{
	switch (msg.messageType)
	{
		case MESSAGE_TYPE_GAME_INFO:
			to_json(j, *static_cast<const GameInfoMessage*>(&msg));
			break;
		case MESSAGE_TYPE_WORLD_UPDATE:
			to_json(j, *static_cast<const WorldUpdateMessage*>(&msg));
			break;
		case MESSAGE_TYPE_TICK:
			to_json(j, *static_cast<const TickMessage*>(&msg));
			break;
		case MESSAGE_TYPE_BOT_SPAWN:
			to_json(j, *static_cast<const BotSpawnMessage*>(&msg));
			break;
		case MESSAGE_TYPE_BOT_KILL:
			to_json(j, *static_cast<const BotKillMessage*>(&msg));
			break;
		case MESSAGE_TYPE_BOT_MOVE:
			to_json(j, *static_cast<const BotMoveMessage*>(&msg));
			break;
		case MESSAGE_TYPE_FOOD_SPAWN:
			to_json(j, *static_cast<const FoodSpawnMessage*>(&msg));
			break;
		case MESSAGE_TYPE_FOOD_CONSUME:
			to_json(j, *static_cast<const FoodConsumeMessage*>(&msg));
			break;
		case MESSAGE_TYPE_FOOD_DECAY:
			to_json(j, *static_cast<const FoodDecayMessage*>(&msg));
			break;
		case MESSAGE_TYPE_PLAYER_INFO:
			to_json(j, *static_cast<const PlayerInfoMessage*>(&msg));
			break;
	}
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::GameInfoMessage &msg)
{
	j = json {
		{"t", "GameInfo"},
		{"world_size_x", msg.world_size_x},
		{"world_size_y", msg.world_size_y},
		{"food_decay_per_frame", msg.food_decay_per_frame}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::WorldUpdateMessage &msg)
{
	json bots = json::object();
	for (auto& bot: msg.bots)
	{
		bots[std::to_string(bot.guid)] = bot;
	}

	json food = json::object();
	for (auto& item: msg.food)
	{
		food[std::to_string(item.guid)] = item;
	}

	j = json {
		{"t", "WorldUpdate"},
		{"bots", bots},
		{"food", food}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::BotItem &item)
{
	j = json {
		{"id", item.guid},
		{"name", item.name},
		{"segment_radius", item.segment_radius},
		{"color", item.color},
		{"snake_segments", item.segments},
		{"heading", 0}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::SnakeSegmentItem &item)
{
	j = json{
		{"pos_x", item.pos().x()},
		{"pos_y", item.pos().y()}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::FoodItem &item)
{
	j = json {
		{"id", item.guid},
		{"pos_x", item.pos().x()},
		{"pos_y", item.pos().y()},
		{"value", item.value}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::TickMessage &msg)
{
	j = json {
		{"t", "Tick"},
		{"frame_id", msg.frame_id}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::BotSpawnMessage &msg)
{
	j = json {
		{"t", "BotSpawn"},
		{"bot", msg.bot}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::BotKillMessage &msg)
{
	j = json {
		{"t", "BotKill"},
		{"killer_id", msg.killer_id},
		{"victim_id", msg.victim_id}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::BotMoveMessage &msg)
{
	j = json {
		{"t", "BotMove"},
		{"items", msg.items}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::BotMoveItem &item)
{
	j = json {
		{"bot_id", item.bot_id},
		{"segment_data", item.new_segments},
		{"length", item.current_length},
		{"segment_radius", item.current_segment_radius}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::FoodSpawnMessage &msg)
{
	j = json {
		{"t", "FoodSpawn"},
		{"items", msg.new_food}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::FoodConsumeMessage &msg)
{
	j = json {
		{"t", "FoodConsume"},
		{"items", msg.items}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::FoodConsumeItem &item)
{
	j = json {
		{"food_id", item.food_id},
		{"bot_id", item.bot_id}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::FoodDecayMessage &msg)
{
	j = json {
		{"t", "FoodDecay"},
		{"items", msg.food_ids}
	};
}

void MsgPackProtocol::to_json(nlohmann::json &j, const MsgPackProtocol::PlayerInfoMessage &msg)
{
	j = json {
		{"t", "PlayerInfo"},
		{"player_id", msg.player_id}
	};
}
