#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <functional>

namespace sfev
{
	// helper using for shorter types 
	using EventCallback = std::function<void(const sf::Event& event)>;

	template<typename T>
	using EventCallbackMap = std::unordered_map<T, EventCallback>;

	using CstEv = const sf::Event&;

	// this class handles subtyped events like keyboard or mouse events
	// the unpack function allows to get relevant information from the processed event
	template<typename T>
	class SubTypeManager
	{
	private:
		EventCallbackMap<T> callmap_;
		std::function<T(const sf::Event&)> unpack_;

	public:
		SubTypeManager(std::function<T(const sf::Event&)> unpack) 
			: unpack_(unpack)
		{ }

		~SubTypeManager() = default;

		void processEvent(const sf::Event& event) const
		{
			T sub_value = unpack_(event);
			auto it(callmap_.find(sub_value));
			if (it != callmap_.end())
			{
				// call its associated callback
				(it->second)(event);
			}
		}

		void addCallback(const T& sub_value, EventCallback callback)
		{
			callmap_[sub_value] = callback;
		}
	};

	class EventMap
	{
	private:
		SubTypeManager<sf::Keyboard::Key> key_pressed_manager_;
		SubTypeManager<sf::Keyboard::Key> key_released_manager_;
		SubTypeManager<sf::Mouse::Button> mouse_pressed_manager_;
		SubTypeManager<sf::Mouse::Button> mouse_released_manager_;
		EventCallbackMap<sf::Event::EventType> event_callmap_;

	public:
		EventMap(bool use_builtin_helpers = true)
			: key_pressed_manager_([](const sf::Event& event) { return event.key.code; }),
			key_released_manager_([](const sf::Event& event) { return event.key.code; }),
			mouse_pressed_manager_([](const sf::Event& event) { return event.mouseButton.button; }),
			mouse_released_manager_([](const sf::Event& event) { return event.mouseButton.button; })
		{
			if (use_builtin_helpers)
			{
				// register key events built in callbacks
				this->addEventCallback(sf::Event::EventType::KeyPressed, [&](const sf::Event& event) { key_pressed_manager_.processEvent(event); });
				this->addEventCallback(sf::Event::EventType::KeyReleased, [&](const sf::Event& event) { key_released_manager_.processEvent(event); });
				this->addEventCallback(sf::Event::EventType::MouseButtonPressed, [&](const sf::Event& event) { mouse_pressed_manager_.processEvent(event); });
				this->addEventCallback(sf::Event::EventType::MouseButtonReleased, [&](const sf::Event& event) { mouse_released_manager_.processEvent(event); });
			}
		}

		// attaches new callback to an event
		void addEventCallback(sf::Event::EventType type, EventCallback callback)
		{
			event_callmap_[type] = callback;
		}

		// adds a key pressed callback
		void addKeyPressedCallback(sf::Keyboard::Key key_code, EventCallback callback)
		{
			key_pressed_manager_.addCallback(key_code, callback);
		}

		// adds a key released callback
		void addKeyReleasedCallback(sf::Keyboard::Key key_code, EventCallback callback)
		{
			key_released_manager_.addCallback(key_code, callback);
		}

		// adds a mouse pressed callback
		void addMousePressedCallback(sf::Mouse::Button button, EventCallback calback)
		{
			mouse_pressed_manager_.addCallback(button, calback);
		}

		// adds a mouse released callback
		void addMouseReleasedCallback(sf::Mouse::Button button, EventCallback callback)
		{
			mouse_released_manager_.addCallback(button, callback);
		}

		// runs the callback associated with the event
		void executeCallback(const sf::Event& e, EventCallback fallback = nullptr) const
		{
			auto it(event_callmap_.find(e.type));
			if (it != event_callmap_.end())
			{
				// call its associated callback
				(it->second)(e);
			}
			else if (fallback)
			{
				fallback(e);
			}
		}

		// remove a callback
		void removeCallback(sf::Event::EventType type)
		{
			// if event type is registered
			auto it(event_callmap_.find(type));
			if (it != event_callmap_.end())
			{
				// remove its associated callback
				event_callmap_.erase(it);
			}
		}
	};

	//	this class handles any type of event and call its associated callback if any.
	//	to process key event in a more convenient way its using a KeyManager 
	
	class EventManager
	{
	private:
		sf::Window& window_;
		EventMap event_map_;

	public:
		EventManager(sf::Window& window, bool use_builtin_helpers) 
			: window_(window), event_map_(use_builtin_helpers)
		{ }

		// calls events attached callback
		void processEvents(EventCallback fallback = nullptr) const
		{
			// iterate over events
			sf::Event event;
			while (window_.pollEvent(event))
			{
				event_map_.executeCallback(event, fallback);
			}
		}

		// attaches new callback to an event
		void addEventCallback(sf::Event::EventType type, EventCallback callback)
		{
			event_map_.addEventCallback(type, callback);
		}

		// removes a callback
		void removeCallback(sf::Event::EventType type)
		{ 
			event_map_.removeCallback(type);
		}

		// adds a key pressed callback
		void addKeyPressedCallback(sf::Keyboard::Key key, EventCallback callback)
		{
			event_map_.addKeyPressedCallback(key, callback);
		}

		// adds a key released callback
		void addKeyReleasedCallback(sf::Keyboard::Key key, EventCallback callback)
		{
			event_map_.addKeyReleasedCallback(key, callback);
		}

		// adds a mouse pressed callback
		void addMousePressedCallback(sf::Mouse::Button button, EventCallback callback)
		{
			event_map_.addMousePressedCallback(button, callback);
		}

		// adds a mouse released callback
		void addMouseReleasedCallback(sf::Mouse::Button button, EventCallback callback)
		{
			event_map_.addMouseReleasedCallback(button, callback);
		}

		sf::Window& getWindow() { return window_; }

		sf::Vector2f getFloatMousePosition() const
		{
			const sf::Vector2i mouse_position = sf::Mouse::getPosition(window_);
			return { static_cast<float>(mouse_position.x), static_cast<float>(mouse_position.y) };
		}

		sf::Vector2i getMousePosition() const
		{
			return sf::Mouse::getPosition(window_);
		}
	};
}
#endif // !EVENTMANAGER_H
