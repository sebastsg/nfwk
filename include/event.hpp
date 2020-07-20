#pragma once

#include <functional>
#include <vector>
#include <queue>
#include <unordered_set>

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

	template<typename... T>
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

template<typename... T>
class event {
public:

	using handler_function = std::function<void(T...)>;

	event() : id{ internal::add_event() } {
		
	}

	event(const event&) = delete;

	event(event&& that) noexcept
		: id{ std::move(that.id) }, handlers{ std::move(that.handlers) }, forward_events{ std::move(that.forward_events) } {
	
	}

	event& operator=(const event&) = delete;

	event& operator=(event&& that) noexcept {
		std::swap(id, that.id);
		std::swap(handlers, that.handlers);
		std::swap(forward_events, that.forward_events);
		return *this;
	}

	~event() {
		internal::remove_event(id);
	}

	[[nodiscard]] event_listener listen(const handler_function& handler) {
		if (!handler) {
			return {};
		}
		const int listener_id{ internal::add_event_listener(id) };
		for (auto& existing_handler : handlers) {
			if (existing_handler.first == -1) {
				existing_handler = { listener_id, handler };
				return { id, listener_id };
			}
		}
		handlers.emplace_back(listener_id, handler);
		return { id, listener_id };
	}

	template<typename... Args>
	void emit(Args... args) const {
		for (const auto& handler : handlers) {
			if (internal::is_event_listener(id, handler.first) && handler.second) {
				handler.second(std::forward<T>(args)...);
			}
		}
		for (const auto& forward_event : forward_events) {
			forward_event->emit(std::forward<T>(args)...);
		}
	}

	int total_listeners() const {
		return static_cast<int>(handlers.size());
	}

	void start_forwarding_to(event& event) {
		forward_events.insert(&event);
	}

	void stop_forwarding_to(event& event) {
		forward_events.erase(&event);
	}

private:

	int id{ -1 };
	std::vector<std::pair<int, handler_function>> handlers;
	std::unordered_set<event*> forward_events;

};

template<typename... T>
class event_queue {
public:

	event_queue() = default;
	event_queue(const event_queue&) = delete;

	event_queue(event_queue&& that) noexcept : messages{ std::move(that.messages) } {
	
	}

	event_queue& operator=(const event_queue&) = delete;

	event_queue& operator=(event_queue&& that) noexcept {
		std::swap(messages, that.messages);
		return *this;
	}

	template<typename... Args>
	void emplace(Args... args) {
		messages.emplace(std::forward<Args>(args)...);
	}
	
	void all(const std::function<void(T...)>& handler) {
		if (handler) {
			while (!messages.empty()) {
				std::apply(handler, messages.front());
				messages.pop();
			}
		}
	}

	void emit(const event<T...>& event_) {
		while (!messages.empty()) {
			std::apply([&](auto&&... args) {
				event_.emit(std::forward<T>(args)...);
			}, messages.front());
			messages.pop();
		}
	}

	size_t size() const {
		return messages.size();
	}

private:

	std::queue<std::tuple<T...>> messages;

};

}
