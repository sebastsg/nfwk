export module nfwk.core:events;

import std.core;
import :event_internal;
import :event_listener;

export namespace nfwk {

template<typename... T>
class event {
public:

	using handler_function = std::function<void(T...)>;

	event() : id{ add_event() } {}

	event(const event&) = delete;

	event(event&& that) noexcept
		: id{ std::move(that.id) }, handlers{ std::move(that.handlers) }, forward_events{ std::move(that.forward_events) } {}

	event& operator=(const event&) = delete;

	event& operator=(event&& that) noexcept {
		std::swap(id, that.id);
		std::swap(handlers, that.handlers);
		std::swap(forward_events, that.forward_events);
		return *this;
	}

	~event() {
		if (id.has_value()) {
			remove_event(id.value());
		}
	}

	event_listener listen(const handler_function& handler) {
		if (!handler || !id.has_value()) {
			return {};
		}
		if (const auto listener_id = add_event_listener(id.value())) {
			for (auto& existing_handler : handlers) {
				if (!existing_handler.first.has_value()) {
					existing_handler = { listener_id, handler };
					return { id, listener_id };
				}
			}
			handlers.emplace_back(listener_id.value(), handler);
			return { id, listener_id };
		}
		return {};
	}

	template<typename... Args>
	void emit(Args... args) const {
		if (!id.has_value()) {
			return;
		}
		for (const auto& handler : handlers) {
			if (handler.first.has_value() && handler.second && is_event_listener(id.value(), handler.first.value())) {
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

	void start_forwarding_to(event& event_) {
		forward_events.insert(&event_);
	}

	void stop_forwarding_to(event& event_) {
		forward_events.erase(&event_);
	}

private:

	std::optional<int> id;
	std::vector<std::pair<std::optional<int>, handler_function>> handlers;
	std::unordered_set<event*> forward_events;

};

}
