export module nfwk.core:event_internal;

import std.core;

export namespace nfwk {

class event_state {
public:

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

}

namespace nfwk {
std::vector<event_state> event_states;
}

export namespace nfwk {

int add_event() {
	for (std::size_t i{ 0 }; i < event_states.size(); i++) {
		if (!event_states[i].event_exists && !event_states[i].has_listeners()) {
			event_states[i] = { true, {} };
			return static_cast<int>(i);
		}
	}
	event_states.push_back({ true, {} });
	return static_cast<int>(event_states.size()) - 1;
}

void remove_event(int event_id) {
	if (event_id >= 0 && event_id < static_cast<int>(event_states.size())) {
		event_states[event_id].event_exists = false;
	}
}

std::optional<int> add_event_listener(int event_id) {
	if (event_id < 0 || event_id >= static_cast<int>(event_states.size()) || !event_states[event_id].event_exists) {
		//warning("events", "Trying to add event listener to non-existing event {}", event_id);
		return std::nullopt;
	}
	auto& state = event_states[event_id];
	state.listeners.push_back(true);
	return static_cast<int>(state.listeners.size()) - 1;
}

void remove_event_listener(int event_id, int listener_id) {
	if (event_id < 0 || event_id >= static_cast<int>(event_states.size())) {
		return;
	}
	auto& state = event_states[event_id];
	if (listener_id < 0 || listener_id >= static_cast<int>(state.listeners.size())) {
		return;
	}
	state.listeners[listener_id] = false;
}

bool is_event_listener(int event_id, int listener_id) {
	if (event_id < 0 || listener_id < 0 || event_id >= static_cast<int>(event_states.size())) {
		return false;
	}
	const auto& state = event_states[event_id];
	if (!state.event_exists || listener_id >= static_cast<int>(state.listeners.size())) {
		return false;
	}
	return state.listeners[listener_id];
}

}
