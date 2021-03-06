#pragma once

#include "Game.h"
#include "Json/JsonParser.h"

namespace Parser
{
	void parseImage(Game& game, const rapidjson::Value& elem);
}
