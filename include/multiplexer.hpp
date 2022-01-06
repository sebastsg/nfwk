#pragma once

#include <chrono>
#include <ctime>
#include <thread>
#include <vector>
#include <atomic>

namespace nfwk {

class timespec_timer {
public:

	timespec_timer();

	std::time_t get_ns() const;
	std::chrono::milliseconds get_ms() const;

private:

	std::timespec start_time;

};

class worker {
public:

	enum class state { running, idle, done };
	enum class priority { high, normal, low };

	worker() = default;
	worker(const worker&) = delete;
	worker(worker&&) = delete;

	virtual ~worker() = default;

	worker& operator=(const worker&) = delete;
	worker& operator=(worker&&) = delete;

	virtual state update() = 0;

};

class basic_multiplexer {
public:

	basic_multiplexer() = default;
	basic_multiplexer(const basic_multiplexer&) = delete;
	basic_multiplexer(basic_multiplexer&&) = delete;

	virtual ~basic_multiplexer() = default;

	basic_multiplexer& operator=(const basic_multiplexer&) = delete;
	basic_multiplexer& operator=(basic_multiplexer&&) = delete;

	void update();
	bool can_handle_work(std::chrono::milliseconds ms) const;
	void add_worker(std::unique_ptr<worker> worker);

private:

	std::vector<std::unique_ptr<worker>> workers;
	std::atomic<std::chrono::milliseconds> last_ms{};

};

class thread_multiplexer : public basic_multiplexer {
public:

	thread_multiplexer();
	
	~thread_multiplexer() override;

private:

	std::thread thread;

};

class multiplexer_list {
public:

	multiplexer_list();
	multiplexer_list(const multiplexer_list&) = delete;
	multiplexer_list(multiplexer_list&&) = delete;

	~multiplexer_list() = default;

	multiplexer_list& operator=(const multiplexer_list&) = delete;
	multiplexer_list& operator=(multiplexer_list&&) = delete;

	void add_worker(std::unique_ptr<worker> worker, worker::priority priority);
	void run();

private:

	std::chrono::milliseconds required_work_for_priority(worker::priority priority) const;

	std::vector<std::unique_ptr<basic_multiplexer>> multiplexers;

};

}
