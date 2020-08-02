#include "loop.hpp"
#include "scripts/script.hpp"
#include "objects/objects.hpp"
#include "editor.hpp"
#include "debug.hpp"
#include "imgui/imgui_platform.hpp"

#if ENABLE_GRAPHICS
#include "graphics/window.hpp"
#endif

#if ENABLE_WASAPI
#include "audio/wasapi.hpp"
#endif

#include "network/network.hpp"

#include <ctime>

extern void configure();
extern void start();

namespace no {

loop_frame_counter::loop_frame_counter() {
	run_timer.start();
}

void loop_frame_counter::next_frame() {
	delta_time = static_cast<double>(new_time - old_time) / 1000000.0;
	old_time = new_time;
	new_time = run_timer.microseconds();
	frames_this_second++;
	if (new_time - time_last_second >= 1000000) {
		time_last_second = new_time;
		frames_last_second = frames_this_second;
		frames_this_second = 0;
	}
	frame_count++;
}

long long loop_frame_counter::ticks() const {
	return run_timer.microseconds();
}

long long loop_frame_counter::frames() const {
	return frame_count;
}

long long loop_frame_counter::current_fps() const {
	return frames_last_second;
}

double loop_frame_counter::average_fps() const {
	return static_cast<double>(frame_count) / static_cast<double>(ticks());
}

double loop_frame_counter::delta() const {
	return delta_time;
}

static struct {

	int ticks_per_second{ 60 };
	int max_update_count{ 5 };
	loop_frame_counter frame_counter;
	draw_synchronization synchronization{ draw_synchronization::if_updated };

#if ENABLE_AUDIO
	audio_endpoint* audio{ nullptr };
#endif

	std::vector<program_state*> states;
	std::vector<window*> windows;
	std::optional<int> imgui_window_index;

	std::vector<program_state*> states_to_stop;

	// Updated when program_state::update() is about to be called.
	program_state* current_state{ nullptr };

	long long redundant_texture_binds_this_frame{ 0 };

	event<> post_configure;
	event<> pre_exit;

} loop;

event<>& post_configure_event() {
	return loop.post_configure;
}

event<>& pre_exit_event() {
	return loop.pre_exit;
}

static int state_index(const program_state* state) {
	for (size_t i{ 0 }; i < loop.states.size(); i++) {
		if (loop.states[i] == state) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

static void update_windows() {
	for (auto state : loop.states) {
		loop.current_state = state;
		const auto index = state_index(state);
#if ENABLE_GRAPHICS
		auto window = loop.windows[index];
		window->poll();
#endif
		ui::start_frame();
		state->update();
		if (loop.imgui_window_index.has_value() && loop.imgui_window_index.value() == index) {
			debug::menu::update();
		}
		ui::end_frame();
		loop.current_state = nullptr;
	}
}

static void draw_windows() {
#if ENABLE_GRAPHICS
	for (auto state : loop.states) {
		loop.current_state = state;
		auto window = loop.windows[state_index(state)];
		window->clear();
		state->draw();
		ui::draw();
		window->swap();
		loop.current_state = nullptr;
	}
#endif
}

static void destroy_stopped_states() {
	for (auto state : loop.states_to_stop) {
		if (const int index{ state_index(state) }; index != -1) {
			const bool closing{ !loop.states[index]->has_next_state() };
			if (closing) {
#if ENABLE_AUDIO
				loop.audio->stop_all_players();
#endif
			}
			INFO_X("main", "About to stop state: " << typeid(*loop.states[index]).name());
			delete loop.states[index];
			loop.states.erase(loop.states.begin() + index);
			if (closing) {
#if ENABLE_GRAPHICS
				if (loop.imgui_window_index.has_value() && loop.imgui_window_index.value() == index) {
					ui::destroy();
					loop.imgui_window_index = std::nullopt;
				}
				INFO_X("main", "About to close the window associated with the closed state.");
				delete loop.windows[index];
				loop.windows.erase(loop.windows.begin() + index);
#endif
#if ENABLE_AUDIO
				loop.audio->clear_players();
#endif
			}
		}
	}
	loop.states_to_stop.clear();
}

static void create_state_generic(const internal::make_state_function& make_state) {
	if (!make_state) {
		return;
	}
	const auto specification = make_state();
	INFO_X("main", "Created state: " << typeid(*specification.state).name());
	loop.states.emplace_back(specification.state);
	if (specification.imgui) {
		ui::create(specification.state->window(), "calibril.ttf", 18);
		loop.imgui_window_index = static_cast<int>(loop.states.size()) - 1;
	}
}

draw_synchronization get_draw_synchronization() {
	return loop.synchronization;
}

void set_draw_synchronization(draw_synchronization synchronization) {
	loop.synchronization = synchronization;
}

const loop_frame_counter& frame_counter() {
	return loop.frame_counter;
}

program_state::program_state() {
#if ENABLE_GRAPHICS
	window_close = window().close.listen([this] {
		stop();
	});
#endif
}

program_state::~program_state() {
	create_state_generic(make_next_state);
}

void program_state::stop() {
	loop.states_to_stop.push_back(this);
}

#if ENABLE_GRAPHICS

window& program_state::window() const {
	if (const int index{ state_index(this) }; index >= 0) {
		return *loop.windows[index];
	} else {
		return *loop.windows.back(); // likely called from constructor, so back() should be correct
	}
}

keyboard& program_state::keyboard() const {
	return window().keyboard;
}

mouse& program_state::mouse() const {
	return window().mouse;
}

#endif

#if ENABLE_AUDIO

audio_endpoint& program_state::audio() const {
	return *loop.audio;
}

#endif

loop_frame_counter& program_state::frame_counter() {
	return loop.frame_counter;
}

long long program_state::redundant_texture_binds_this_frame() {
	return loop.redundant_texture_binds_this_frame;
}

void program_state::change_state(const internal::make_state_function& make_state) {
	make_next_state = make_state;
	loop.states_to_stop.push_back(this);
}

bool program_state::has_next_state() const {
	return make_next_state.operator bool();
}

program_state* program_state::current() {
	if (!loop.current_state) {
		WARNING_LIMIT_X("graphics", "No state is being updated or drawn.", 10);
	}
	ASSERT(loop.current_state);
	return loop.current_state;
}

namespace internal {

void create_state(const std::string& title, int width, int height, int samples, const make_state_function& make_state) {
	loop.windows.emplace_back(new window{ title, width, height, samples });
	create_state_generic(make_state);
}

void create_state(const std::string& title, int width, int height, const make_state_function& make_state) {
	loop.windows.emplace_back(new window{ title, width, height });
	create_state_generic(make_state);
}

void create_state(const std::string& title, int samples, const make_state_function& make_state) {
	loop.windows.emplace_back(new window{ title, samples });
	create_state_generic(make_state);
}

void create_state(const std::string& title, const make_state_function& make_state) {
	loop.windows.emplace_back(new window{ title });
	create_state_generic(make_state);
}

int run_main_loop() {
	MESSAGE_X("main", "Configuring...");
	configure();
	MESSAGE_X("main", "Done configuring.");
	loop.post_configure.emit();

	MESSAGE_X("main", "Initializing all systems...");
	debug::internal::start_debug();
	internal::initialize_editor();
	internal::initialize_scripts();
	internal::initialize_objects();

#if ENABLE_WASAPI
	loop.audio = new wasapi::audio_device{};
#endif

#if ENABLE_NETWORK
	start_network();
#endif

	MESSAGE_X("main", "Done initializing all systems.");

	start();

	MESSAGE_X("main", "Program has been started.");

	long long next_tick{ loop.frame_counter.ticks() };

	while (!loop.states.empty()) {
		int update_count{ 0 };
		const long long frame_skip{ 1000000 / loop.ticks_per_second };
		const long long reference_ticks{ loop.frame_counter.ticks() };

		bool updated{ false };
		while (reference_ticks - next_tick > frame_skip && update_count < loop.max_update_count) {
			update_windows();
			next_tick += frame_skip;
			update_count++;
			updated = true;
		}

		if (updated || loop.synchronization == draw_synchronization::always) {
#if TRACK_REDUNDANT_BINDS
			const auto redundant_texture_binds = debug::get_redundant_texture_bind_calls();
#endif
			draw_windows();
#if TRACK_REDUNDANT_BINDS
			loop.redundant_texture_binds_this_frame = debug::get_redundant_texture_bind_calls() - redundant_texture_binds;
#endif
			loop.frame_counter.next_frame();
		}

		destroy_stopped_states();
	}
	MESSAGE_X("main", "Loop has reached its end.");
	destroy_main_loop();
	return 0;
}

void destroy_main_loop() {
	MESSAGE_X("main", "Cleaning up everything.");
	for (auto state : loop.states) {
		loop.states_to_stop.push_back(state);
	}
	destroy_stopped_states();
	loop.pre_exit.emit();
#if ENABLE_NETWORK
	stop_network();
#endif
#if ENABLE_AUDIO
	delete loop.audio;
	loop.audio = nullptr;
#endif
	debug::internal::stop_debug();
}

}

}
