#ifndef WINDOWCONTEXTHANDLER_H
#define WINDOWCONTEXTHANDLER_H

#include <SFML/Graphics.hpp>
#include "render/viewport_handler.hpp"
#include "common/event_manager.hpp"
#include "common/utils.hpp"

class WindowContextHandler;

class RenderContext
{
private:
	sf::RenderWindow& window_;
	ViewPortHandler viewport_handler_;

	friend class WindowContextHandler;

public:
	explicit 
		RenderContext(sf::RenderWindow& window) 
		: window_(window), viewport_handler_(toVector2f(window.getSize()))
	{ }

	void setFocus(sf::Vector2f focus)
	{
		viewport_handler_.setFocus(focus);
	}

	void setZoom(float zoom)
	{
		viewport_handler_.setZoom(zoom);
	}

	void registerCallbacks(sfev::EventManager& event_manager)
	{
		event_manager.addEventCallback(sf::Event::Closed, [&](sfev::CstEv) { window_.close(); });
		event_manager.addKeyPressedCallback(sf::Keyboard::Escape, [&](sfev::CstEv) { window_.close(); });
		event_manager.addMousePressedCallback(sf::Mouse::Left, [&](sfev::CstEv) {
			viewport_handler_.click(event_manager.getFloatMousePosition()); 
		});
		event_manager.addMouseReleasedCallback(sf::Mouse::Left, [&](sfev::CstEv) {
			viewport_handler_.unclick(); 
		});
		event_manager.addEventCallback(sf::Event::MouseMoved, [&](sfev::CstEv) {
			viewport_handler_.setMousePosition(event_manager.getFloatMousePosition());
		});
		event_manager.addEventCallback(sf::Event::MouseWheelScrolled, [&](sfev::CstEv e) {
			viewport_handler_.wheelZoom(e.mouseWheelScroll.delta);
		});
	}

	void drawDirect(const sf::Drawable& drawable)
	{
		window_.draw(drawable);
	}

	void draw(const sf::Drawable& drawable, sf::RenderStates render_states = {})
	{
		render_states.transform = viewport_handler_.getTransform();
		window_.draw(drawable, render_states);
	}

	void clear(sf::Color color = sf::Color::Black)
	{
		window_.clear(color);
	}

	void display()
	{
		window_.display();
	}
};

class WindowContextHandler
{
private:
	sf::RenderWindow window_;
	sfev::EventManager event_manager_;
	RenderContext render_context_;

public:
	WindowContextHandler(const std::string& window_name, 
		sf::Vector2u window_size,
		int32_t window_style = sf::Style::Default) 
		: window_(sf::VideoMode(window_size.x, window_size.y), window_name, window_style),
		event_manager_(window_, true),
		render_context_(window_)
	{
		window_.setFramerateLimit(60);
		render_context_.registerCallbacks(event_manager_);
	}

	[[nodiscard]]
	sf::Vector2u getWindowSize() const
	{
		return window_.getSize();
	}

	void processEvents()
	{
		event_manager_.processEvents();
	}
	
	bool isRunning() const
	{
		return window_.isOpen();
	}

	bool run()
	{
		processEvents();
		return isRunning();
	}

	sfev::EventManager& getEventManager()
	{
		return event_manager_;
	}

	RenderContext& getRenderContext()
	{
		return render_context_;
	}

	sf::Vector2f getWorldMousePosition() const
	{
		return render_context_.viewport_handler_.getMouseWorldPosition();
	}

	void setFramerateLimit(uint32_t framerate)
	{
		window_.setFramerateLimit(framerate);
	}
};

#endif // !WINDOWCONTEXTHANDLER_H
