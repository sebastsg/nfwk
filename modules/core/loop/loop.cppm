module;

#include "assert.hpp"

export module nfwk.core:loop;

import std.core;
import std.memory;
export import :loop.frame_rate_controller;
export import :loop.state;
export import :loop.component;

export namespace nfwk {

int run_main_loop() {
	loop_state loop;
	frame_rate_controller frame_rate;
	while (!loop.states.empty()) {
		frame_rate.set_reference_here();
		while (frame_rate.ready_for_update()) {
			loop.update();
			frame_rate.on_updated();
		}
		if (frame_rate.ready_for_draw()) {
			//const auto redundant_texture_binds = debug::get_redundant_texture_bind_calls();
			loop.draw();
			//loop.redundant_texture_binds_this_frame = debug::get_redundant_texture_bind_calls() - redundant_texture_binds;
			frame_rate.next_frame();
		}
		loop.destroy_stopped_states();
	}
	return 0;
}

}

#if 0
class draw_debug_metrics {
public:
	long long redundant_texture_binds_this_frame{ 0 };
};
draw_synchronization get_draw_synchronization() {
	return loop.synchronization;
}
void set_draw_synchronization(draw_synchronization synchronization) {
	loop.synchronization = synchronization;
}
event<>& post_configure_event() {
	return loop.post_configure;
}
event<>& pre_exit_event() {
	return loop.pre_exit;
}
const loop_frame_counter& frame_counter() {
	return loop.frame_counter;
}

class imgui_loop_component : public loop_component {
public:

	imgui_loop_component(program_state& state) : loop_component{ state } {
		ui::create(state->window(), "calibril.ttf", 18);
		imgui_window_index = static_cast<int>(loop.states.size()) - 1;
	}

	~imgui_loop_component() override {
		if (imgui_window_index.has_value() && imgui_window_index.value() == index) {
			ui::destroy();
			imgui_window_index = std::nullopt;
		}
	}

	void before() override {
		ui::start_frame();
	}

	void after() override {
		if (imgui_window_index.has_value() && imgui_window_index.value() == index) {
			debug::menu::update();
		}
		ui::end_frame();
	}

	void after_draw() override {
		ui::draw();
	}

private:

	std::optional<int> imgui_window_index;

};

class window_loop_component : public loop_component {
public:

	window_loop_component(program_state& state) : loop_component{ state } {
		window_close = state_window->close.listen([this] {
			state.stop();
		});
	}

	~window_loop_component() override {
		info("draw", "About to close the window associated with the closed state.");
	}

	void before() override {
		state_window->poll();
	}

	void before_draw() override {
		state_window->clear();
	}

	void after_draw() override {
		state_window->swap();
	}

private:

	window state_window;
	event_listener window_close;

};

class audio_loop_component : public loop_component {
public:

	audio_loop_component(program_state& state) : loop_component{ state } {
		endpoint = std::make_unique<wasapi::audio_device>();
	}

private:

	std::unique_ptr<audio_endpoint> endpoint;

};

#endif