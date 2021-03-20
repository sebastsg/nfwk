#pragma once

#include "loop_frame_counter.hpp"
#include "log.hpp"
#include "assert.hpp"

namespace nfwk {

class fixed_frame_rate_controller {
public:

	fixed_frame_rate_controller(frame_counter& counter) : counter{ counter }, next_tick{ counter.ticks() } {}
	fixed_frame_rate_controller(const fixed_frame_rate_controller&) = delete;
	fixed_frame_rate_controller(fixed_frame_rate_controller&&) = delete;

	~fixed_frame_rate_controller() = default;

	fixed_frame_rate_controller& operator=(const fixed_frame_rate_controller&) = delete;
	fixed_frame_rate_controller& operator=(fixed_frame_rate_controller&&) = delete;

	void set_reference_here() {
		update_count = 0;
		frame_skip = 1000000 / ticks_per_second;
		reference_ticks = counter.ticks();
		updated = false;
	}

	bool ready_for_update() const {
		return reference_ticks - next_tick > frame_skip && update_count < max_update_count;
	}

	void on_updated() {
		next_tick += frame_skip;
		update_count++;
		updated = true;
	}

	bool ready_for_new_frame() const {
		return updated;
	}

	void next_frame() {
		ASSERT(ready_for_new_frame());
		counter.next_frame();
	}

private:

	frame_counter& counter;

	long long next_tick{ 0 };
	int ticks_per_second{ 60 };
	int max_update_count{ 5 };

	bool updated{ false };
	int update_count{ 0 };
	long long frame_skip{ 0 };
	long long reference_ticks{ 0 };

};

}
