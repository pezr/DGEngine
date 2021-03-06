#include "ParseImage.h"
#include "Image.h"
#include "GameUtils.h"
#include "ParseUtils.h"

namespace Parser
{
	using namespace rapidjson;

	void parseImage(Game& game, const Value& elem)
	{
		if (isValidString(elem, "id") == false || isValidString(elem, "texture") == false)
		{
			return;
		}

		auto texture = game.Resources().getTexture(elem["texture"].GetString());
		if (texture == nullptr)
		{
			return;
		}

		auto image = std::make_shared<Image>(*texture);

		if (elem.HasMember("textureRect"))
		{
			sf::IntRect rect(0, 0, game.WindowSize().x, game.WindowSize().y);
			image->setTextureRect(getIntRect(elem, "textureRect", rect));
		}

		auto anchor = getAnchor(elem, "anchor");
		image->setAnchor(anchor);
		auto pos = getVector2f<sf::Vector2f>(elem, "position");
		if (getBool(elem, "relativeCoords", true) == true)
		{
			auto size = image->Size();
			GameUtils::setAnchorPosSize(anchor, pos, size, game.RefSize(), game.MinSize());
			if (game.StretchToFit() == false)
			{
				GameUtils::setAnchorPosSize(anchor, pos, size, game.MinSize(), game.WindowSize());
			}
		}
		image->Position(pos);
		image->Visible(getBool(elem, "visible", true));

		image->setColor(getColor(elem, "color", sf::Color::White));

		game.Resources().addDrawable(elem["id"].GetString(), image);
	}
}
