module;

#include "assert.hpp"

export module nfwk.core:loop.frame_rate_controller;

import :loop.frame_counter;

export namespace nfwk {

// 'always' should only used to test performance, and requires swap_interval::immediate.
enum class draw_synchronization { always, if_updated };

class frame_rate_controller {
public:

	frame_rate_controller() {
		next_tick = frame_counter.ticks();
	}
	
	void set_reference_here() {
		update_count = 0;
		frame_skip = 1000000 / ticks_per_second;
		reference_ticks = frame_counter.ticks();
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

	bool ready_for_draw() const {
		return updated || synchronization == draw_synchronization::always;
	}

	void next_frame() {
		ASSERT(ready_for_draw());
		frame_counter.next_frame();
	}

private:

	loop_frame_counter frame_counter;

	long long next_tick{ 0 };
	int ticks_per_second{ 60 };
	int max_update_count{ 5 };
	draw_synchronization synchronization{ draw_synchronization::if_updated };

	bool updated{ false };
	int update_count{ 0 };
	long long frame_skip{ 0 };
	long long reference_ticks{ 0 };

};

}
