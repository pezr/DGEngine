#include "ParseCursor.h"
#include "ParseAnimation.h"
#include "ParseUtils.h"

namespace Parser
{
	using namespace rapidjson;

	sf::Vector2f getOrigin(const Value& elem, float width, float height)
	{
		if (elem.HasMember("origin") && elem["origin"].IsString() && elem["origin"].GetString() == std::string("center"))
		{
			return sf::Vector2f(std::round(width / 2.f), std::round(height / 2.f));
		}
		else
		{
			return getVector2f<sf::Vector2f>(elem, "origin");
		}
	}

	void parseCursor(Game& game, const Value& elem)
	{
		game.Window().setMouseCursorVisible(getBool(elem, "show"));

		if (getBool(elem, "pop") == true)
		{
			game.Resources().popCursor();
			game.updateMouse();
		}

		if (elem.HasMember("texture") || elem.HasMember("file"))
		{
			auto cursor = parseAnimationObj(game, elem);
			if (cursor == nullptr)
			{
				return;
			}
			auto size = cursor->Size();
			cursor->setOrigin(getOrigin(elem, size.x, size.y));
			game.Resources().addCursor(cursor);
			game.updateMouse();
		}
		else
		{
			game.Resources().addCursor(nullptr);
		}
	}
}
