#pragma once

#include "debug.hpp"

#include <functional>
#include <vector>
#include <queue>

namespace no {

namespace internal {

int add_event();
void remove_event(int event_id);
int add_event_listener(int event_id);
void remove_event_listener(int event_id, int listener_id);
bool is_event_listener(int event_id, int listener_id);

}

class event_listener {
public:

	template<typename T>
	friend class event;

	event_listener() = default;
	event_listener(const event_listener&) = delete;
	event_listener(event_listener&&) noexcept;

	~event_listener();

	event_listener& operator=(const event_listener&) = delete;
	event_listener& operator=(event_listener&&) noexcept;

	bool exists() const;
	void stop();

private:

	event_listener(int event_id, int listener_id);

	int event_id{ -1 };
	int listener_id{ -1 };

};

template<typename T>
class event {
public:

	using handler_function = std::function<void(const T&)>;

	event() {
		id = internal::add_event();
	}

	event(const event&) = delete;
	event(event&& that) : id{ std::move(that.id) }, handlers{ std::move(that.handlers) } {

	}

	event& operator=(const event&) = delete;
	event& operator=(event&& that) {
		std::swap(id, that.id);
		std::swap(handlers, that.handlers);
		return *this;
	}

	~event() {
		internal::remove_event(id);
	}

	[[nodiscard]] event_listener listen(handler_function&& handler) {
		if (!handler) {
			return {};
		}
		int listener_id{ internal::add_event_listener(id) };
		for (auto& existing_handler : handlers) {
			if (existing_handler.first == -1) {
				existing_handler = { listener_id, handler };
				return { id, listener_id };
			}
		}
		handlers.emplace_back(listener_id, handler);
		return { id, listener_id };
	}

	void emit(const T& event) const {
		for (auto& handler : handlers) {
			if (internal::is_event_listener(id, handler.first) && handler.second) {
				handler.second(event);
			}
		}
	}

	template<typename... Args>
	void emit(Args... args) const {
		emit(T{ std::forward<Args>(args)... });
	}

	int total_listeners() const {
		return (int)handlers.size();
	}

private:

	int id{ -1 };
	std::vector<std::pair<int, handler_function>> handlers;

};

template<typename T>
class event_queue {
public:

	event_queue() = default;

	event_queue(const event_queue&) = delete;

	event_queue(event_queue&& that) : messages{ std::move(that.messages) } {
	
	}

	event_queue& operator=(const event_queue&) = delete;

	event_queue& operator=(event_queue&& that) {
		std::swap(messages, that.messages);
		return *this;
	}

	void move_and_push(T&& message) {
		messages.push(std::move(message));
	}

	template<typename... Args>
	void emplace_and_push(Args... args) {
		messages.emplace(T{ std::forward<Args>(args)... });
	}

	void all(const std::function<void(const T&)>& function) {
		if (function) {
			while (!messages.empty()) {
				function(messages.front());
				messages.pop();
			}
		}
	}

	void emit(const event<T>& listener) {
		while (!messages.empty()) {
			listener.emit(messages.front());
			messages.pop();
		}
	}

	size_t size() const {
		return messages.size();
	}

private:

	std::queue<T> messages;

};

}
