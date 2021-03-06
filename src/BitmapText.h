#pragma once

#include "BitmapFont.h"
#include "DrawableText.h"
#include <memory>
#include <SFML/Graphics.hpp>

class BitmapText : public DrawableText
{
private:
	std::string text;
	std::shared_ptr<BitmapFont> font;
	sf::Vector2f pos;
	sf::Vector2f drawPos;
	sf::Vector2f size;
	HorizontalAlign horizAlign{ HorizontalAlign::Left };
	VerticalAlign vertAlign{ VerticalAlign::Bottom };
	Anchor anchor{ Anchor::Top | Anchor::Left };
	sf::Color color{ sf::Color::White };
	float lineSpacing{ .0f };
	bool visible{ true };

	void calcDrawPos();
	void calcSize();

public:
	BitmapText(const std::string& text_, const std::shared_ptr<BitmapFont> font_)
		: text(text_), font(font_)
	{
		calcSize();
		calcDrawPos();
		lineSpacing = (float)font->getNewLine();
	}

	virtual ~BitmapText() {}

	void setFont(const std::shared_ptr<BitmapFont>& font_) { font = font_; }

	virtual std::string getText() const { return text; }

	virtual void setAnchor(const Anchor anchor_)
	{
		if (anchor != anchor_)
		{
			anchor = anchor_;
			calcDrawPos();
		}
	}
	virtual void updateSize(const Game& game);

	virtual void setText(const std::string& string)
	{
		text = string;
		calcSize();
		calcDrawPos();
	}

	virtual void setColor(const sf::Color& color_) { color = color_; }

	virtual const sf::Vector2f& DrawPosition() const { return drawPos; }
	virtual const sf::Vector2f& Position() const { return pos; }
	virtual void Position(const sf::Vector2f& position)
	{
		pos = position;
		calcDrawPos();
	}
	virtual sf::Vector2f Size() const { return size; }
	virtual void Size(const sf::Vector2f& size) {}

	virtual void setLineSpacing(unsigned lineSpacing_)
	{
		lineSpacing = (float)lineSpacing_;
		calcSize();
	}
	virtual sf::FloatRect getLocalBounds() const { return sf::FloatRect(drawPos, size); }
	virtual sf::FloatRect getGlobalBounds() const { return sf::FloatRect(drawPos, size); }

	virtual void setHorizontalAlign(const HorizontalAlign align)
	{
		if (horizAlign != align)
		{
			horizAlign = align;
			calcDrawPos();
		}
	}

	virtual void setVerticalAlign(const VerticalAlign align)
	{
		if (vertAlign != align)
		{
			vertAlign = align;
			calcDrawPos();
		}
	}

	virtual bool Visible() const { return visible; }
	virtual void Visible(bool visible_) { visible = visible_; }

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	void update(Game& game) {}

	virtual Variable getProperty(const std::string& prop) const;
};
