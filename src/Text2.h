#pragma once

#include "DrawableText.h"
#include <memory>
#include "UIObject.h"

class Text2 : public UIObject
{
private:
	std::unique_ptr<DrawableText> text;
	std::string format;
	std::vector<std::string> bindings;

	void setBindingHelper(const std::string& binding);

public:
	Text2(std::unique_ptr<DrawableText> text_) : text(std::move(text_)) {}

	DrawableText* getDrawableText() { return text.get(); }

	std::string getText() const { return text->getText(); }
	void setText(std::unique_ptr<DrawableText> text_) { text = std::move(text_); }
	void setText(const std::string& text_) { text->setText(text_); }

	sf::FloatRect getLocalBounds() const { return text->getLocalBounds(); }
	sf::FloatRect getGlobalBounds() const { return text->getGlobalBounds(); }

	void setBinding(const std::string& binding);
	void setBinding(const std::vector<std::string>& bindings_);
	void setColor(const sf::Color& color) { text->setColor(color); }
	void setHorizontalAlign(const HorizontalAlign align) { text->setHorizontalAlign(align); }
	void setLineSpacing(unsigned lineSpacing) { text->setLineSpacing(lineSpacing); }
	void setFormat(const std::string& format_) { format = format_; }
	void setVerticalAlign(const VerticalAlign align) { text->setVerticalAlign(align); }

	virtual void setAnchor(const Anchor anchor) { text->setAnchor(anchor); }
	virtual void updateSize(const Game& game) { text->updateSize(game); }

	virtual const sf::Vector2f& DrawPosition() const { return text->DrawPosition(); }
	virtual const sf::Vector2f& Position() const { return text->Position(); }
	virtual void Position(const sf::Vector2f& pos) { text->Position(pos); }
	virtual sf::Vector2f Size() const { return text->Size(); }
	virtual void Size(const sf::Vector2f& size) { text->Size(size); }

	virtual bool Visible() const { return text->Visible(); }
	virtual void Visible(bool visible) { text->Visible(visible); }

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const { target.draw(*text, states); }

	virtual void update(Game& game);

	virtual Variable getProperty(const std::string& prop) const { return text->getProperty(prop); }
};
