#include "loop.hpp"
#include "script.hpp"

#if ENABLE_WINDOW
#include "window.hpp"
#endif

#if ENABLE_WASAPI
#include "wasapi.hpp"
#endif

#include "network.hpp"

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
	return (double)frame_count / (double)ticks();
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

	std::vector<program_state*> states_to_stop;

	long long redundant_bind_calls_this_frame{ 0 };

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
#if ENABLE_WINDOW
		auto window = loop.windows[state_index(state)];
		window->poll();
#endif
		state->update();
	}
}

static void draw_windows() {
#if ENABLE_WINDOW
	for (auto state : loop.states) {
		auto window = loop.windows[state_index(state)];
		window->clear();
		state->draw();
		window->swap();
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
			delete loop.states[index];
			loop.states.erase(loop.states.begin() + index);
			if (closing) {
#if ENABLE_WINDOW
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

program_state::program_state() {
#if ENABLE_WINDOW
	window_close = window().close.listen([this] {
		loop.states_to_stop.push_back(this);
	});
#endif
}

program_state::~program_state() {
	if (make_next_state) {
		loop.states.emplace_back(make_next_state());
	}
}

void program_state::stop() {
	loop.states_to_stop.push_back(this);
}

#if ENABLE_WINDOW

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

const loop_frame_counter& program_state::frame_counter() const {
	return loop.frame_counter;
}

loop_frame_counter& program_state::frame_counter() {
	return loop.frame_counter;
}

void program_state::set_synchronization(draw_synchronization synchronization) {
	loop.synchronization = synchronization;
}

long long program_state::redundant_bind_calls_this_frame() {
	return loop.redundant_bind_calls_this_frame;
}

void program_state::change_state(const internal::make_state_function& make_state) {
	make_next_state = make_state;
	loop.states_to_stop.push_back(this);
}

bool program_state::has_next_state() const {
	return make_next_state.operator bool();
}

std::string current_local_time_string() {
	const time_t now{ std::time(nullptr) };
	tm local_time;
#if PLATFORM_WINDOWS
	localtime_s(&local_time, &now);
#else
	local_time = *localtime(&now);
#endif
	char buffer[64];
	std::strftime(buffer, 64, "%X", &local_time);
	return buffer;
}

std::string curent_local_date_string() {
	const time_t now{ std::time(nullptr) };
	tm local_time;
#if PLATFORM_WINDOWS
	localtime_s(&local_time, &now);
#else
	local_time = *localtime(&now);
#endif
	char buffer[64];
	std::strftime(buffer, 64, "%Y.%m.%d", &local_time);
	return buffer;
}

namespace internal {

void create_state(const std::string& title, int width, int height, int samples, const make_state_function& make_state) {
	loop.windows.emplace_back(new window{ title, width, height, samples });
	loop.states.emplace_back(make_state());
}

void create_state(const std::string& title, int width, int height, const make_state_function& make_state) {
	loop.windows.emplace_back(new window{ title, width, height });
	loop.states.emplace_back(make_state());
}

void create_state(const std::string& title, int samples, const make_state_function& make_state) {
	loop.windows.emplace_back(new window{ title, samples });
	loop.states.emplace_back(make_state());
}

void create_state(const std::string& title, const make_state_function& make_state) {
#if ENABLE_WINDOW
	loop.windows.emplace_back(new window{ title });
#else
	loop.windows.emplace_back(nullptr);
#endif
	loop.states.emplace_back(make_state());
}

int run_main_loop() {
	configure();
	loop.post_configure.emit();

	initialize_scripts();

#if ENABLE_WASAPI
	loop.audio = new wasapi::audio_device{};
#endif

#if ENABLE_NETWORK
	start_network();
#endif

	start();

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
			const auto current_redundant_bind_calls{ total_redundant_bind_calls() };
			draw_windows();
			loop.redundant_bind_calls_this_frame = total_redundant_bind_calls() - current_redundant_bind_calls;
			loop.frame_counter.next_frame();
		}

		destroy_stopped_states();
	}
	destroy_main_loop();
	return 0;
}

void destroy_main_loop() {
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
}

}

}
