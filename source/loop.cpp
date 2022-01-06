#include "loop.hpp"
#include "nfwk.hpp"
#include "subprogram.hpp"
#include "log.hpp"
#include "timer.hpp"
#include "event.hpp"

#include <functional>

//void run() {
	//initialize_sprites();
	//internal::initialize_scripts();
	//objects::internal::initialize();
	//load_variables();
	//objects::load_classes();
//}

namespace nfwk {

loop::~loop() {
	message(core::log, "Exiting.");
}

void loop::run() {
	if (inside_run) {
		error(core::log, "Already running.");
		return;
	}
	inside_run = true;
	move_new_subprograms();
	while (is_running()) {
		move_new_subprograms();
		update();
		destroy_stopped_subprograms();
	}
	inside_run = false;
}

int loop::current_fps() const {
	return counter.current_fps();
}

float loop::delta() const {
	return static_cast<float>(counter.delta());
}

void loop::add(std::unique_ptr<subprogram> subprogram) {
	subprogram->owning_loop = this;
	new_subprograms.emplace_back(std::move(subprogram));
}

void loop::remove(const subprogram& subprogram) {
	subprograms_to_stop.push_back(&subprogram);
}

void loop::destroy_stopped_subprograms() {
	if (subprograms_to_stop.empty()) {
		return;
	}
	for (int i{ 0 }; i < static_cast<int>(subprograms.size()); i++) {
		for (const auto* subprogram_to_stop : subprograms_to_stop) {
			if (subprograms[i].get() == subprogram_to_stop) {
				subprograms.erase(subprograms.begin() + i);
				i--;
				break;
			}
		}
	}
	subprograms_to_stop.clear();
}

void loop::move_new_subprograms() {
	if (new_subprograms.empty()) {
		return;
	}
	for (auto& new_subprogram : new_subprograms) {
		subprograms.emplace_back(std::move(new_subprogram));
	}
	new_subprograms.clear();
}

void loop::update() {
	on_begin_update.emit();
	for (auto& subprogram : subprograms) {
		subprogram->update();
	}
	on_end_update.emit();
	on_begin_frame.emit();
	on_end_frame.emit();
	counter.next_frame();
}

bool loop::is_running() const {
	return !subprograms.empty() || !new_subprograms.empty();
}

fixed_time_step_loop::fixed_time_step_loop() : frame_rate_controller{ counter } {

}

void fixed_time_step_loop::update() {
	frame_rate_controller.set_reference_here();
	while (frame_rate_controller.ready_for_update()) {
		on_begin_update.emit();
		for (auto& subprogram : subprograms) {
			subprogram->update();
		}
		on_end_update.emit();
		frame_rate_controller.on_updated();
	}
	if (frame_rate_controller.ready_for_new_frame()) {
		on_begin_frame.emit();
		on_end_frame.emit();
		frame_rate_controller.next_frame();
	}
}

const fixed_frame_rate_controller& fixed_time_step_loop::get_frame_rate_controller() const {
	return frame_rate_controller;
}

}
