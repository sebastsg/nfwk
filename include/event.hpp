#pragma once

#include <functional>
#include <vector>
#include <queue>
#include <unordered_set>
#include <optional>
#include <limits>

namespace nfwk::internal {
int add_event();
void remove_event(int event_id);
int add_event_listener(int event_id);
void remove_event_listener(int event_id, int listener_id);
bool is_event_listener(int event_id, int listener_id);
}

namespace nfwk {

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

namespace priority {

static_assert(sizeof(int) >= 4, "The sections constant is made with >= 32 bits in mind.");

constexpr int lowest{ std::numeric_limits<int>::min() };
constexpr int highest{ std::numeric_limits<int>::max() };
constexpr int sections{ 16384 };
constexpr int per_section{ highest / sections };
constexpr int normal{ 0 };

}

class priority_manager {
public:

	priority_manager(int max_sections, int per_section) {
		sections_used.resize(max_sections);
	}

	bool create_section(int section, std::string_view name) {
		if (sections_used[section]) {
			return false;
		} else {
			sections_used[section] = true;
			sections[section] = name;
			return true;
		}
	}

	bool delete_section(int section) {
		if (!sections_used[section]) {
			return false;
		} else {
			sections_used[section] = false;
			sections.erase(section);
			return true;
		}
	}

private:

	std::vector<bool> sections_used;
	std::unordered_map<int, std::string> sections;

};

[[nodiscard]] inline bool create_priority_section(int section, std::string_view name) {
	return false;
}

[[nodiscard]] inline bool delete_priority_section(int section) {
	return false;
}

[[nodiscard]] constexpr int make_priority(int section, int priority) {
	return section * priority::per_section + priority;
}

template<typename... T>
class event {
public:

	using handler_function = std::function<void(T...)>;

	event() : id{ internal::add_event() } {}
	event(const event&) = delete;
	event(event&& that) noexcept : id{ std::move(that.id) }, handlers{ std::move(that.handlers) }, forward_events{ std::move(that.forward_events) } {}

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

	[[nodiscard]] event_listener listen(const handler_function& handler, int priority = priority::normal) {
		if (!handler) {
			return {};
		}
		const auto listener_id = internal::add_event_listener(id);
		if (auto unused_handler = find_unused_handler()) {
			*unused_handler = { listener_id, priority, handler };
			sort_listeners_by_priority();
			return { id, listener_id };
		} else {
			handlers.emplace_back(listener_id, priority, handler);
			sort_listeners_by_priority();
			return { id, listener_id };
		}
	}

	template<typename... Args>
	void emit(Args... args) const {
		for (auto& handler : handlers) {
			if (handler.is_listening_to(id)) {
				handler.handler(std::forward<T>(args)...);
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

	class event_handler {
	public:

		std::optional<int> listener_id;
		int priority{ 0 };
		handler_function handler;

		event_handler(int listener_id, int priority, handler_function handler)
			: listener_id{ listener_id }, priority{ priority }, handler{ std::move(handler) } {}

		bool is_listening_to(int event_id) const {
			return listener_id.has_value() && internal::is_event_listener(event_id, listener_id.value()) && handler;
		}

	};

	void sort_listeners_by_priority() {
		std::sort(handlers.begin(), handlers.end(), [](const event_handler& a, const event_handler& b) {
			return a.priority > b.priority;
		});
	}

	event_handler* find_unused_handler() {
		for (auto& handler : handlers) {
			if (!handler.listener_id.has_value()) {
				return &handler;
			}
		}
		return nullptr;
	}

	int id{ -1 };
	std::vector<event_handler> handlers;
	std::unordered_set<event*> forward_events; // unordered_set is 40 bytes... make into vector?

};

template<typename... T>
class event_queue {
public:

	event_queue() = default;
	event_queue(const event_queue&) = delete;
	event_queue(event_queue&& that) noexcept : messages{ std::move(that.messages) } {}

	~event_queue() = default;

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

	std::size_t size() const {
		return messages.size();
	}

private:

	std::queue<std::tuple<T...>> messages;

};

}
