#include "event.hpp"
#include "debug.hpp"

namespace no::internal {

struct event_state {
	
	bool event_exists{ false };
	std::vector<bool> listeners;

	bool has_listeners() const {
		for (const bool listener : listeners) {
			if (listener) {
				return true;
			}
		}
		return false;
	}

};

static struct {
	std::vector<event_state> events;
} data;

int add_event() {
	for (size_t i{ 0 }; i < data.events.size(); i++) {
		if (!data.events[i].event_exists && !data.events[i].has_listeners()) {
			data.events[i] = { true, {} };
			return static_cast<int>(i);
		}
	}
	data.events.push_back({ true, {} });
	return static_cast<int>(data.events.size()) - 1;
}

void remove_event(int event_id) {
	if (event_id >= 0 && event_id < static_cast<int>(data.events.size())) {
		data.events[event_id].event_exists = false;
	}
}

int add_event_listener(int event_id) {
	if (event_id < 0 || event_id >= static_cast<int>(data.events.size()) || !data.events[event_id].event_exists) {
		WARNING("Trying to add event listener to non-existing event " << event_id);
		return -1;
	}
	auto& event = data.events[event_id];
	event.listeners.push_back(true);
	return static_cast<int>(event.listeners.size()) - 1;
}

void remove_event_listener(int event_id, int listener_id) {
	if (event_id < 0 || event_id >= static_cast<int>(data.events.size())) {
		return;
	}
	auto& event{ data.events[event_id] };
	if (listener_id < 0 || listener_id >= static_cast<int>(event.listeners.size())) {
		return;
	}
	event.listeners[listener_id] = false;
}

bool is_event_listener(int event_id, int listener_id) {
	if (event_id < 0 || listener_id < 0 || event_id >= static_cast<int>(data.events.size())) {
		return false;
	}
	const auto& event = data.events[event_id];
	if (!event.event_exists || listener_id >= static_cast<int>(event.listeners.size())) {
		return false;
	}
	return event.listeners[listener_id];
}

}

namespace no {

event_listener::event_listener(int event_id, int listener_id) : event_id{ event_id }, listener_id{ listener_id } {

}

event_listener::event_listener(event_listener&& that) noexcept {
	std::swap(event_id, that.event_id);
	std::swap(listener_id, that.listener_id);
}

event_listener::~event_listener() {
	stop();
}

event_listener& event_listener::operator=(event_listener&& that) noexcept {
	std::swap(event_id, that.event_id);
	std::swap(listener_id, that.listener_id);
	return *this;
}

bool event_listener::exists() const {
	return internal::is_event_listener(event_id, listener_id);
}

void event_listener::stop() {
	internal::remove_event_listener(event_id, listener_id);
	event_id = -1;
	listener_id = -1;
}

}
