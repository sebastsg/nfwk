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

using make_state_function = std::function<program_state*()>;

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

	const loop_frame_counter& frame_counter() const;
	loop_frame_counter& frame_counter();
	bool has_next_state() const;

protected:

	template<typename T>
	void change_state() {
		change_state([] {
			return new T{};
		});
	}

	void set_synchronization(draw_synchronization synchronization);
	long long redundant_bind_calls_this_frame();

	void stop();

private:

	void change_state(const internal::make_state_function& make_state);

	internal::make_state_function make_next_state;
	event_listener window_close;

};

event<>& post_configure_event();
event<>& pre_exit_event();

template<typename T>
void create_state(const std::string& title, int width, int height, int samples) {
	internal::create_state(title, width, height, samples, [] { return new T{}; });
}

template<typename T>
void create_state(const std::string& title, int width, int height) {
	internal::create_state(title, width, height, [] { return new T{}; });
}

template<typename T>
void create_state(const std::string& title, int samples) {
	internal::create_state(title, samples, [] { return new T{}; });
}

template<typename T>
void create_state(const std::string& title) {
	internal::create_state(title, [] { return new T{}; });
}

std::string current_local_time_string();
std::string curent_local_date_string();

}
