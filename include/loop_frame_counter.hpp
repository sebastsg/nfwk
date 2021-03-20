#pragma once

#include "timer.hpp"

namespace nfwk {

class frame_counter {
public:

	frame_counter() {
		run_timer.start();
	}

	frame_counter(const frame_counter&) = delete;
	frame_counter(frame_counter&&) = delete;

	~frame_counter() = default;

	frame_counter& operator=(const frame_counter&) = delete;
	frame_counter& operator=(frame_counter&&) = delete;

	void next_frame() {
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

	long long ticks() const {
		return run_timer.microseconds();
	}

	long long frames() const {
		return frame_count;
	}

	int current_fps() const {
		return frames_last_second;
	}

	double average_fps() const {
		return static_cast<double>(frame_count) / static_cast<double>(ticks());
	}

	double delta() const {
		return delta_time;
	}

private:

	timer run_timer;
	long long frame_count{ 0 };
	long long old_time{ 0 };
	long long new_time{ 0 };
	long long time_last_second{ 0 };
	int frames_this_second{ 0 };
	int frames_last_second{ 0 };
	double delta_time{ 0.0 };

};

}
