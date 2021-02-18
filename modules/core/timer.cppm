export module nfwk.core:timer;

import :platform;

export namespace nfwk {

class timer {
public:

	void start() {
		started = true;
		paused = false;
		paused_time = 0;
		start_time = performance_counter();
		ticks_per_second = performance_frequency();
	}

	void stop() {
		started = false;
		paused = false;
		paused_time = 0;
		start_time = 0;
		ticks_per_second = 0;
	}

	void pause() {
		if (started && !paused) {
			paused = true;
			const long long now{ performance_counter() };
			paused_time = ticks_to_microseconds(now - start_time);
		}
	}

	void resume() {
		paused = false;
		paused_time = 0;
	}

	bool is_paused() const {
		return paused;
	}

	bool has_started() const {
		return started;
	}

	long long microseconds() const {
		if (!started) {
			return 0;
		} else if (paused) {
			return paused_time;
		} else {
			const long long now{ performance_counter() };
			return ticks_to_microseconds(now - start_time);
		}
	}

	long long milliseconds() const {
		return microseconds() / 1000;
	}

	long long seconds() const {
		return microseconds() / 1000 / 1000;
	}

	long long minutes() const {
		return microseconds() / 1000 / 1000 / 60;
	}

	long long hours() const {
		return microseconds() / 1000 / 1000 / 60 / 60;
	}

	long long days() const {
		return microseconds() / 1000 / 1000 / 60 / 60 / 24;
	}

	void go_back_in_time(long long milliseconds) {
		if (started) {
			return;
		}
		if (paused) {
			paused_time -= microseconds_to_ticks(milliseconds * 1000);
		} else {
			start_time -= microseconds_to_ticks(milliseconds * 1000);
		}
	}

private:

	long long microseconds_to_ticks(long long microseconds) const {
		return microseconds * ticks_per_second / 1000000;
	}

	long long ticks_to_microseconds(long long ticks) const {
		return ticks * 1000000 / ticks_per_second;
	}

	bool paused{ false };
	bool started{ false };

	long long paused_time{ 0 };
	long long start_time{ 0 };
	long long ticks_per_second{ 0 };

};

}
