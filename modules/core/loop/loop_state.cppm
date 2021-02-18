module;

#include "assert.hpp"
#include "nfwk.hpp"

export module nfwk.core:loop.state;

import std.core;
import std.memory;
import :loop.component;
import :events;
import :log;

export namespace nfwk {

class program_state;

class core_events {
public:
	event<> post_configure;
	event<> pre_exit;
};

class loop_state {
public:

	loop_state() {
		message("core", "Configuring...");
		configure();
		message("core", "Done configuring.");
		//loop.post_configure.emit();
		message("core", "Initializing all systems...");
		log::start_logging();
		//internal::initialize_editor();
		//initialize_sprites();
		//internal::initialize_scripts();
		//objects::internal::initialize();
		//load_variables();
		//objects::load_classes();
		//start_network();
		message("core", "Done initializing all systems.");
		start();
		message("core", "Entering loop.");
	}

	~loop_state() {
		message("core", "Destroying loop.");
		for (auto& state : states) {
			stop_state(*state);
		}
		destroy_stopped_states();
		//pre_exit.emit();
		//stop_network();
		log::stop_logging();
	}

	program_state* get_current_state() {
		if (!current_state) {
			warning("core", "No state is being updated or drawn.");
		}
		ASSERT(current_state);
		return current_state;
	}

	void update();
	void draw();

	void destroy_stopped_states() {
		//for (auto& state : states_to_stop) {
			//const bool closing{ !state->has_next_state() };
			//if (closing) {
				//audio->stop_all_players();
			//}
			//info("main", "About to stop state: {}", typeid(*states[index]).name());
			//states.erase(states.begin() + index);
			//if (closing) {
				//if (imgui_window_index.has_value() && imgui_window_index.value() == index) {
				//	ui::destroy();
				//	imgui_window_index = std::nullopt;
				//}
				//info("core", "About to close the window associated with the closed state.");
				//delete windows[index];
				//windows.erase(loop.windows.begin() + index);
				//audio->clear_players();
			//}
		//}
		states_to_stop.clear();
	}

	void stop_state(const program_state& state) {
		for (std::size_t i{ 0 }; i < states.size(); i++) {
			if (states[i].get() == &state) {
				states_to_stop.emplace_back(std::move(states[i]));
				states.erase(states.begin() + i);
				break;
			}
		}
	}

//private:

	// Updated when program_state::update() is about to be called.
	program_state* current_state{ nullptr };

	std::vector<std::unique_ptr<program_state>> states;
	std::vector<std::unique_ptr<program_state>> states_to_stop;

};

}

namespace nfwk {

std::shared_ptr<loop_state> loop;

}

export namespace nfwk {

class program_state {
public:

	program_state() {
		//info("core", "Created state: {}", typeid(*this).name());
	}

	program_state(const program_state&) = delete;
	program_state(program_state&&) = delete;

	virtual ~program_state() {
		loop->states.emplace_back(make_next_state());
	}

	program_state& operator=(const program_state&) = delete;
	program_state& operator=(program_state&&) = delete;

	virtual void update() = 0;
	virtual void draw() = 0;

	template<typename State>
	static void create_state() {
		loop->states.emplace_back(std::make_unique<State>());
	}

	template<typename State>
	void change_state() {
		change_state([] {
			return std::make_unique<State>();
		});
	}

	void change_state(const std::function<std::unique_ptr<program_state>()>& make_state) {
		make_next_state = make_state;
		stop();
	}

	bool has_next_state() const {
		return make_next_state.operator bool();
	}

	static program_state* current() {
		return loop->get_current_state();
	}

	void pre_update() {
		for (auto& component : components) {
			component->before();
		}
	}
	
	void post_update() {
		for (auto& component : components) {
			component->after();
		}
	}

	void pre_draw() {
		for (auto& component : components) {
			component->before_draw();
		}
	}
	
	void post_draw() {
		for (auto& component : components) {
			component->after_draw();
		}
	}

protected:

	void stop() {
		loop->stop_state(*this);
	}

private:

	std::function<std::unique_ptr<program_state>()> make_next_state;
	std::vector<std::unique_ptr<loop_component>> components;

#if 0
public:
	loop_frame_counter& frame_counter() {
		return loop->frame_counter;
	}
	window& window() const {
		if (const int index{ loop->state_index(this) }; index >= 0) {
			return *loop.windows[index];
		} else {
			return *loop.windows.back(); // likely called from constructor, so back() should be correct
		}
	}
	keyboard& keyboard() const {
		return window().keyboard;
	}
	mouse& mouse() const {
		return window().mouse;
	}
	audio_endpoint& audio() const {
		return *loop.audio;
	}
	long long redundant_texture_binds_this_frame() const {
		return loop->redundant_texture_binds_this_frame;
	}
#endif

};

void loop_state::update() {
	for (auto& state : states) {
		current_state = state.get();
		//windows[index]->poll();
		//ui::start_frame();
		state->pre_update();
		state->update();
		state->post_update();
		//if (imgui_window_index.has_value() && imgui_window_index.value() == index) {
		//	debug::menu::update();
		//}
		//ui::end_frame();
		current_state = nullptr;
	}
}

void loop_state::draw() {
	for (auto& state : states) {
		current_state = state.get();
		//auto window = windows[state_index(state)];
		//window->clear();
		state->pre_draw();
		state->draw();
		state->post_draw();
		//ui::draw();
		//window->swap();
		current_state = nullptr;
	}
}

}
