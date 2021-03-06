#include "Game.h"
#include "GameUtils.h"
#include "Json/JsonUtils.h"
#include "Parser/ParseVariable.h"
#include "ReverseIterable.h"
#include "SFMLUtils.h"
#include "Utils.h"

using Utils::str2int;

Game::~Game()
{
	resourceManager = ResourceManager();
	if (window.isOpen() == true)
	{
		window.close();
	}
}

void Game::init()
{
	if (window.isOpen() == true)
	{
		return;
	}
#ifdef __ANDROID__
	window.create(sf::VideoMode::getDesktopMode(), title);
	size = window.getSize();
	oldSize = size;
#else
	oldSize = size;
	window.create(sf::VideoMode(size.x, size.y), title);
#endif
	window.setFramerateLimit(framerate);
	if (stretchToFit == true)
	{
		stretchToFit = false;
		StretchToFit(true);
	}
	updateWindowTex();
}

void Game::play()
{
	if (window.isOpen() == false)
	{
		return;
	}

	updateMouse();
	sf::Clock frameClock;

	while (window.isOpen() == true)
	{
		mouseClicked = false;
		mouseDoubleClicked = false;
		mouseMoved = false;
		mouseReleased = false;
		mouseScrolled = false;
		keyboardChar = 0;
		keyPressed.code = sf::Keyboard::Unknown;

		processEvents();
		checkKeyPress();

		window.clear();
		windowTex.clear();

		elapsedTime = frameClock.restart();

		updateEvents();

		resourceManager.clearFinishedSounds();

		if (drawLoadingScreen() == false)
		{
			drawAndUpdate();
			drawCursor();
			drawFadeEffect();
			drawWindow();
		}
	}
}

void Game::processEvents()
{
	sf::Event evt;
	while (window.pollEvent(evt))
	{
		switch (evt.type)
		{
		case sf::Event::Closed:
			onClosed();
			break;
		case sf::Event::Resized:
			onResized(evt.size);
			break;
		case sf::Event::LostFocus:
			onLostFocus();
			break;
		case sf::Event::GainedFocus:
			onGainedFocus();
			break;
		case sf::Event::TextEntered:
			onTextEntered(evt.text);
			break;
		case sf::Event::KeyPressed:
			onKeyPressed(evt.key);
			break;
		case sf::Event::MouseWheelScrolled:
			onMouseWheelScrolled(evt.mouseWheelScroll);
			break;
		case sf::Event::MouseButtonPressed:
			onMouseButtonPressed(evt.mouseButton);
			break;
		case sf::Event::MouseButtonReleased:
			onMouseButtonReleased(evt.mouseButton);
			break;
		case sf::Event::MouseMoved:
			onMouseMoved(evt.mouseMove);
			break;
		default:
			break;
		}
	}
}

void Game::onClosed()
{
	window.close();
}

void Game::onResized(const sf::Event::SizeEvent& evt)
{
	bool needsResize = false;
	sf::Vector2u newSize(evt.width, evt.height);
	if (newSize.x < minSize.x)
	{
		newSize.x = minSize.x;
		needsResize = true;
	}
	if (newSize.y < minSize.y)
	{
		newSize.y = minSize.y;
		needsResize = true;
	}
	if (needsResize == true)
	{
		window.setSize(newSize);
		return;
	}
	oldSize = size;
	size = newSize;

	updateWindowTex();

	auto view = window.getView();
	if (stretchToFit == false)
	{
		view.reset(sf::FloatRect(0.f, 0.f, evt.width, evt.height));
	}
	else
	{
		if (keepAR == true)
		{
			sf::viewStretchKeepAR(view, newSize);
		}
		else
		{
			view.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
		}
	}
	window.setView(view);
	updateSize();
}

void Game::updateSize()
{
	if (loadingScreen != nullptr)
	{
		loadingScreen->updateSize(*this);
	}
	if (fadeInOut != nullptr)
	{
		fadeInOut->updateSize(*this);
	}
	for (auto& res : resourceManager)
	{
		for (auto& obj : res.drawables)
		{
			obj.second->updateSize(*this);
		}
	}
}

void Game::updateWindowTex()
{
	if (stretchToFit == false)
	{
		windowTex.create(size.x, size.y);
		windowTexSize = size;
	}
	else
	{
		windowTex.create(minSize.x, minSize.y);
		windowTexSize = minSize;
	}
	windowTex.setSmooth(smoothScreen);
	windowSprite.setTexture(windowTex.getTexture(), true);
}

void Game::onLostFocus()
{
	paused = (pauseOnFocusLoss == true);
}

void Game::onGainedFocus()
{
	paused = false;
}

void Game::onTextEntered(const sf::Event::TextEvent& evt)
{
	if (evt.unicode < 256)
	{
		keyboardChar = static_cast<char>(evt.unicode);
	}
}

void Game::onKeyPressed(const sf::Event::KeyEvent& evt)
{
	if (disableInput == false)
	{
		keyPressed = evt;
	}
}

void Game::onMouseWheelScrolled(const sf::Event::MouseWheelScrollEvent& evt)
{
	updateMouse();
	mouseWheel = evt;
	mouseWheel.x = mousePosition.x;
	mouseWheel.y = mousePosition.y;
	mouseScrolled = true;
}

void Game::onMouseButtonPressed(const sf::Event::MouseButtonEvent& evt)
{
	mouseButton = evt.button;
	mouseClicked = true;
	mousePressed = true;

	auto mouseClickElapsedTime = mouseClickClock.restart();
	if (mouseClickElapsedTime.asMilliseconds() <= GameUtils::DoubleClickDelay)
	{
		mouseDoubleClicked = true;
	}
}

void Game::onMouseButtonReleased(const sf::Event::MouseButtonEvent& evt)
{
	mouseButton = evt.button;
	mousePressed = false;
	mouseReleased = true;
}

void Game::onMouseMoved(const sf::Event::MouseMoveEvent& evt)
{
	updateMouse(sf::Vector2i(evt.x, evt.y));
	mouseMoved = true;
}

void Game::updateMouse()
{
	updateMouse(sf::Mouse::getPosition(window));
}

void Game::updateMouse(const sf::Vector2i mousePos)
{
	mousePosition = window.mapPixelToCoords(mousePos);
	mousePosition.x = std::round(mousePosition.x);
	mousePosition.y = std::round(mousePosition.y);
	auto cursor = resourceManager.getCursor();
	if (cursor != nullptr)
	{
		cursor->Position(mousePosition);
	}
}

void Game::checkKeyPress()
{
	if (loadingScreen == nullptr && keyPressed.code != sf::Keyboard::Unknown)
	{
		auto action = resourceManager.getKeyboardAction(keyPressed);
		if (action != nullptr)
		{
			eventManager.addBack(action);
		}
	}
}

void Game::updateEvents()
{
	if (paused == false)
	{
		eventManager.update(*this);
	}
}

bool Game::drawLoadingScreen()
{
	if (loadingScreen == nullptr)
	{
		return false;
	}
	windowTex.draw(*loadingScreen);
	drawFadeEffect();
	drawWindow();
	return true;
}

void Game::drawAndUpdate()
{
	for (auto& res : reverse(resourceManager))
	{
		if (res.ignore != IgnoreResource::DrawAndUpdate)
		{
			for (auto it2 = res.drawables.rbegin(); it2 != res.drawables.rend(); ++it2)
			{
				auto& obj = *(it2);
				if (paused == false && res.ignore != IgnoreResource::Update)
				{
					obj.second->update(*this);
				}
			}
		}
	}
	for (auto& res : resourceManager)
	{
		if (res.ignore != IgnoreResource::DrawAndUpdate)
		{
			for (auto& obj : res.drawables)
			{
				windowTex.draw(*obj.second);
			}
		}
	}
}

void Game::drawCursor()
{
	auto cursor = resourceManager.getCursor();
	if (cursor != nullptr)
	{
		cursor->update(*this);
		windowTex.draw(*cursor);
	}
}

void Game::drawFadeEffect()
{
	if (fadeInOut != nullptr)
	{
		windowTex.draw(static_cast<sf::RectangleShape>(*fadeInOut));
	}
}

void Game::drawWindow()
{
	windowTex.display();
	window.draw(windowSprite);
	window.display();
}

void Game::SmoothScreen(bool smooth_)
{
	smoothScreen = smooth_;
	windowTex.setSmooth(smooth_);
}

void Game::StretchToFit(bool stretchToFit_)
{
	if (stretchToFit != stretchToFit_)
	{
		stretchToFit = stretchToFit_;
		if (window.isOpen() == true)
		{
			if (stretchToFit_ == true)
			{
				auto view = window.getView();
				view.reset(sf::FloatRect(0, 0, minSize.x, minSize.y));
				window.setView(view);
				oldSize = size;
				size = minSize;
				stretchToFit = false;
				updateSize();
				size = oldSize;
				stretchToFit = true;
				if (keepAR == true)
				{
					sf::viewStretchKeepAR(view, size);
				}
				else
				{
					view.setViewport(sf::FloatRect(0, 0, 1, 1));
				}
				window.setView(view);
			}
			else
			{
				auto view = window.getView();
				view.setViewport(sf::FloatRect(0, 0, 1, 1));
				window.setView(view);
				oldSize = minSize;
				updateSize();
				view.reset(sf::FloatRect(0, 0, size.x, size.y));
				window.setView(view);
				updateWindowTex();
			}
			updateMouse();
		}
	}
}

void Game::KeepAR(bool keepAR_)
{
	if (keepAR != keepAR_)
	{
		if (window.isOpen() == true && stretchToFit == true)
		{
			auto view = window.getView();
			if (keepAR_ == true)
			{
				sf::viewStretchKeepAR(view, size);
			}
			else
			{
				view.setViewport(sf::FloatRect(0, 0, 1, 1));
			}
			window.setView(view);
			updateSize();
			updateMouse();
		}
		keepAR = keepAR_;
	}
}

void Game::addPlayingSound(const sf::SoundBuffer& obj)
{
	sf::Sound sound(obj);
	sound.setVolume((float)soundVolume);
	resourceManager.addPlayingSound(sound);
}

void Game::addPlayingSound(const sf::SoundBuffer* obj)
{
	if (obj != nullptr)
	{
		addPlayingSound(*obj);
	}
}

std::map<std::string, Variable>::const_iterator Game::findVariable(const std::string& key) const
{
	if ((key.size() > 2) &&
		(key.front() == '%') &&
		(key.back() == '%'))
	{
		return variables.find(key.substr(1, key.size() - 2));
	}
	return variables.end();
}

Variable Game::getVariable(const std::string& key) const
{
	auto it = findVariable(key);
	if (it != variables.cend())
	{
		return it->second;
	}
	return Parser::getVarOrObjectProperty(*this, key);
}

bool Game::getVariableBool(const std::string& key) const
{
	auto it = findVariable(key);
	if (it == variables.cend())
	{
		return false;
	}
	const auto& var = it->second;
	if (var.is<bool>())
	{
		return (var.get<bool>() ? 1L : 0L);
	}
	else if (var.is<int64_t>())
	{
		return var.get<int64_t>() != 0;
	}
	else if (var.is<double>())
	{
		return var.get<double>() != 0.0;
	}
	else if (var.is<std::string>())
	{
		return var.get<std::string>().empty() == false;
	}
	return false;
}

double Game::getVariableDouble(const std::string& key) const
{
	auto it = findVariable(key);
	if (it == variables.cend())
	{
		return false;
	}
	const auto& var = it->second;
	if (var.is<int64_t>())
	{
		return var.get<int64_t>();
	}
	else if (var.is<double>())
	{
		return (long)var.get<double>();
	}
	else if (var.is<bool>())
	{
		return (var.get<bool>() ? 1.0 : 0.0);
	}
	else if (var.is<std::string>())
	{
		return std::atof(var.get<std::string>().c_str());
	}
	return 0.0;
}

int64_t Game::getVariableLong(const std::string& key) const
{
	auto it = findVariable(key);
	if (it == variables.cend())
	{
		return false;
	}
	const auto& var = it->second;
	if (var.is<int64_t>())
	{
		return var.get<int64_t>();
	}
	else if (var.is<double>())
	{
		return (long)var.get<double>();
	}
	else if (var.is<bool>())
	{
		return (var.get<bool>() ? 1L : 0L);
	}
	else if (var.is<std::string>())
	{
		return std::atol(var.get<std::string>().c_str());
	}
	return 0L;
}

std::string Game::getVariableString(const std::string& key) const
{
	if (key.size() <= 2)
	{
		return key;
	}
	else
	{
		if ((key.front() == '%') && (key.back() == '%'))
		{
			auto it = variables.find(key.substr(1, key.size() - 2));
			if (it != variables.end())
			{
				const auto& var = it->second;
				if (var.is<std::string>())
				{
					auto str = var.get<std::string>();
					return VarUtils::toString(Parser::getVarOrObjectProperty(*this, str));
				}
				else if (var.is<int64_t>())
				{
					return std::to_string(var.get<int64_t>());
				}
				else if (var.is<double>())
				{
					return std::to_string(var.get<double>());
				}
				else if (var.is<bool>())
				{
					if (var.get<bool>() == true)
					{
						return std::string("true");
					}
					else
					{
						return std::string("false");
					}
				}
			}
		}
		return VarUtils::toString(Parser::getVarOrObjectProperty(*this, key));
	}
}

void Game::clearVariable(const std::string& key)
{
	if (key.size() > 1)
	{
		if ((key.front() == '%') && (key.back() == '%'))
		{
			auto it = variables.find(key.substr(1, key.size() - 2));
			if (it != variables.end())
			{
				variables.erase(it);
			}
		}
	}
}

void Game::setVariable(const std::string& key, const Variable& value)
{
	variables[key] = value;
}

void Game::saveVariables(const std::string& filePath, const std::vector<std::string>& varNames) const
{
	if (varNames.empty() == true)
	{
		return;
	}
	std::vector<std::pair<std::string, Variable>> variablesToSave;
	for (const auto& name : varNames)
	{
		auto it = variables.find(name);
		if (it != variables.end())
		{
			variablesToSave.push_back(*it);
		}
	}
	if (variablesToSave.empty() == false)
	{
		JsonUtils::saveToFile<decltype(variablesToSave)>(filePath, "variable", variablesToSave);
	}
}

Variable Game::getProperty(const std::string& prop) const
{
	if (prop.size() > 1)
	{
		auto props = Utils::splitString(prop, '.');
		if (props.size() > 0)
		{
			switch (str2int(props[0].c_str()))
			{
			case str2int("framerate"):
				return Variable((int64_t)framerate);
			case str2int("keepAR"):
				return Variable((bool)keepAR);
			case str2int("minSize"):
			{
				if (props.size() > 1)
				{
					if (props[1] == "x")
					{
						return Variable((int64_t)minSize.x);
					}
					else
					{
						return Variable((int64_t)minSize.y);
					}
				}
			}
			break;
			case str2int("musicVolume"):
				return Variable((int64_t)musicVolume);
			case str2int("path"):
				return Variable(path);
			case str2int("refSize"):
			{
				if (props.size() > 1)
				{
					if (props[1] == "x")
					{
						return Variable((int64_t)refSize.x);
					}
					else
					{
						return Variable((int64_t)refSize.y);
					}
				}
			}
			case str2int("saveDir"):
				return Variable(std::string(FileUtils::getSaveDir()));
			case str2int("size"):
			{
				if (props.size() > 1)
				{
					if (props[1] == "x")
					{
						return Variable((int64_t)size.x);
					}
					else
					{
						return Variable((int64_t)size.y);
					}
				}
			}
			break;
			case str2int("smoothScreen"):
				return Variable((bool)smoothScreen);
			case str2int("soundVolume"):
				return Variable((int64_t)soundVolume);
			case str2int("stretchToFit"):
				return Variable((bool)stretchToFit);
			case str2int("title"):
				return Variable(title);
			case str2int("version"):
				return Variable(version);
			}
		}
	}
	return Variable();
}

void Game::setProperty(const std::string& prop, const Variable& val)
{
	if (prop.size() > 1)
	{
		auto props = Utils::splitString(prop, '.');
		if (props.size() > 0)
		{
			switch (str2int(props[0].c_str()))
			{
			case str2int("framerate"):
			{
				if (val.is<int64_t>() == true)
				{
					Framerate((unsigned)val.get<int64_t>());
				}
			}
			break;
			case str2int("keepAR"):
			{
				if (val.is<bool>() == true)
				{
					KeepAR(val.get<bool>());
				}
			}
			break;
			case str2int("musicVolume"):
			{
				if (val.is<int64_t>() == true)
				{
					MusicVolume((unsigned)val.get<int64_t>());
				}
			}
			break;
			case str2int("smoothScreen"):
			{
				if (val.is<bool>() == true)
				{
					SmoothScreen(val.get<bool>());
				}
			}
			break;
			case str2int("soundVolume"):
			{
				if (val.is<int64_t>() == true)
				{
					SoundVolume((unsigned)val.get<int64_t>());
				}
			}
			break;
			case str2int("stretchToFit"):
			{
				if (val.is<bool>() == true)
				{
					StretchToFit(val.get<bool>());
				}
			}
			break;
			case str2int("title"):
			{
				if (val.is<std::string>() == true)
				{
					setTitle(val.get<std::string>());
				}
			}
			break;
			case str2int("version"):
				if (val.is<std::string>() == true)
				{
					setVersion(val.get<std::string>());
				}
			}
		}
	}
}
