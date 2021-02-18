export module nfwk.core:event_listener;

import std.core;
import :event_internal;

export namespace nfwk {

class [[nodiscard]] event_listener {
public:

	event_listener(std::optional<int> event_id, std::optional<int> listener_id)
		: event_id{ event_id }, listener_id{ listener_id } {}

	event_listener() = default;
	event_listener(const event_listener&) = delete;

	event_listener(event_listener&& that) noexcept {
		std::swap(event_id, that.event_id);
		std::swap(listener_id, that.listener_id);
	}

	~event_listener() {
		stop();
	}

	event_listener& operator=(const event_listener&) = delete;

	event_listener& operator=(event_listener&& that) noexcept {
		std::swap(event_id, that.event_id);
		std::swap(listener_id, that.listener_id);
		return *this;
	}

	bool exists() const {
		return event_id.has_value() && listener_id.has_value() && is_event_listener(event_id.value(), listener_id.value());
	}

	void stop() {
		if (exists()) {
			remove_event_listener(event_id.value(), listener_id.value());
			event_id = std::nullopt;
			listener_id = std::nullopt;
		}
	}

private:

	std::optional<int> event_id;
	std::optional<int> listener_id;

};

}
