#include "multiplexer.hpp"

namespace nfwk {

timespec_timer::timespec_timer() {
	std::timespec_get(&start_time, TIME_UTC);
}

std::time_t timespec_timer::get_ns() const {
	const std::time_t ns_before{ start_time.tv_sec * 1000000000 + start_time.tv_nsec };
	std::timespec new_time;
	std::timespec_get(&new_time, TIME_UTC);
	const std::time_t ns_after{ new_time.tv_sec * 1000000000 + new_time.tv_nsec };
	const std::time_t ns_difference{ ns_after - ns_before };
	return ns_difference;
}

std::chrono::milliseconds timespec_timer::get_ms() const {
	return std::chrono::milliseconds{ get_ns() / 1000 / 1000 };
}

void basic_multiplexer::update() {
	timespec_timer timer;
	for (int i{ 0 }; i < static_cast<int>(workers.size()); i++) {
		if (workers[i]->update() == worker::state::done) {
			workers.erase(workers.begin() + i);
			i--;
		}
	}
	last_ms = std::chrono::milliseconds{ timer.get_ms() };
}

bool basic_multiplexer::can_handle_work(std::chrono::milliseconds ms) const {
	return last_ms.load() + ms < std::chrono::milliseconds{ 1000 };
}

void basic_multiplexer::add_worker(std::unique_ptr<worker> worker) {
	workers.emplace_back(std::move(worker));
}

thread_multiplexer::thread_multiplexer() {
	thread = std::thread{ [this] {
		update();
	} };
}

thread_multiplexer::~thread_multiplexer() {
	if (thread.joinable()) {
		thread.join();
	}
}

multiplexer_list::multiplexer_list() {
	multiplexers.emplace_back(std::make_unique<basic_multiplexer>());
}

void multiplexer_list::add_worker(std::unique_ptr<worker> worker, worker::priority priority) {
	for (auto& multiplexer : multiplexers) {
		if (multiplexer->can_handle_work(required_work_for_priority(priority))) {
			multiplexer->add_worker(std::move(worker));
			return;
		}
	}
	multiplexers.emplace_back(std::make_unique<thread_multiplexer>())->add_worker(std::move(worker));
}

void multiplexer_list::run() {
	// todo: better exit condition
	while (!multiplexers.empty()) {
		multiplexers[0]->update();
	}
}

std::chrono::milliseconds multiplexer_list::required_work_for_priority(worker::priority priority) const {
	switch (priority) {
	case worker::priority::high: return std::chrono::milliseconds{ 800 };
	case worker::priority::normal: return std::chrono::milliseconds{ 400 };
	case worker::priority::low: return std::chrono::milliseconds{ 100 };
	}
}

}
