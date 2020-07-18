#pragma once

#include "timer.hpp"
#include "event.hpp"
#include "audio.hpp"

#include <string>
#include <functional>

namespace no {

class window;
class program_state;
class keyboard;
class mouse;

namespace internal {

struct make_state_specification {
	program_state* state{ nullptr };
	bool imgui{ false };
};

using make_state_function = std::function<make_state_specification()>;

void create_state(const std::string& title, int width, int height, int samples, const make_state_function& make_state);
void create_state(const std::string& title, int width, int height, const make_state_function& make_state);
void create_state(const std::string& title, int samples, const make_state_function& make_state);
void create_state(const std::string& title, const make_state_function& make_state);

int run_main_loop();
void destroy_main_loop();

}

class loop_frame_counter {
public:

	loop_frame_counter();

	void next_frame();

	long long ticks() const;
	long long frames() const;
	long long current_fps() const;
	double average_fps() const;
	double delta() const;

private:

	timer run_timer;
	long long frame_count{ 0 };
	long long old_time{ 0 };
	long long new_time{ 0 };
	long long time_last_second{ 0 };
	long long frames_this_second{ 0 };
	long long frames_last_second{ 0 };
	double delta_time{ 0.0 };

};

// 'always' should only used to test performance, and requires swap_interval::immediate.
enum class draw_synchronization { always, if_updated };

draw_synchronization get_draw_synchronization();
void set_draw_synchronization(draw_synchronization synchronization);

const loop_frame_counter& frame_counter();

class program_state {
public:

	program_state();
	program_state(const program_state&) = delete;
	program_state(program_state&&) = delete;

	virtual ~program_state();

	program_state& operator=(const program_state&) = delete;
	program_state& operator=(program_state&&) = delete;

	virtual void update() = 0;

#if ENABLE_WINDOW
	
	virtual void draw() = 0;

	window& window() const;
	keyboard& keyboard() const;
	mouse& mouse() const;

#endif

#if ENABLE_AUDIO

	audio_endpoint& audio() const;

#endif

	loop_frame_counter& frame_counter();
	bool has_next_state() const;

	template<typename T, bool CreateImGui = false>
	void change_state() {
		change_state([] {
			return internal::make_state_specification{ new T{}, CreateImGui };
		});
	}

	static program_state* current();

protected:

	long long redundant_texture_binds_this_frame();

	void stop();

private:

	void change_state(const internal::make_state_function& make_state);

	internal::make_state_function make_next_state;
	event_listener window_close;

};

event<>& post_configure_event();
event<>& pre_exit_event();

template<typename T, bool CreateImGui = false>
void create_state(const std::string& title, int width, int height, int samples) {
	internal::create_state(title, width, height, samples, [] {
		return internal::make_state_specification{ new T{}, CreateImGui };
	});
}

template<typename T, bool CreateImGui = false>
void create_state(const std::string& title, int width, int height) {
	internal::create_state(title, width, height, [] {
		return internal::make_state_specification{ new T{}, CreateImGui };
	});
}

template<typename T, bool CreateImGui = false>
void create_state(const std::string& title, int samples) {
	internal::create_state(title, samples, [] {
		return internal::make_state_specification{ new T{}, CreateImGui };
	});
}

template<typename T, bool CreateImGui = false>
void create_state(const std::string& title) {
	internal::create_state(title, [] {
		return internal::make_state_specification{ new T{}, CreateImGui };
	});
}

}
