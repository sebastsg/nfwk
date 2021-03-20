#pragma once

#include "frame_rate_controller.hpp"

#include <memory>
#include <vector>

namespace nfwk {

class subprogram;
class component;

class loop {
public:

	event<> on_begin_update;
	event<> on_end_update;
	event<> on_begin_frame;
	event<> on_end_frame;

	virtual ~loop();

	virtual bool is_running() const;

	void run();

	int current_fps() const;
	float delta() const;

	void add(std::unique_ptr<subprogram> subprogram);
	void remove(const subprogram& subprogram);

	template<typename Subprogram>
	Subprogram& add() {
		auto subprogram = std::make_unique<Subprogram>(*this);
		auto& subprogram_reference = *subprogram;
		add(std::move(subprogram));
		return subprogram_reference;
	}

protected:

	void destroy_stopped_subprograms();
	void move_new_subprograms();

	frame_counter counter;
	std::vector<std::unique_ptr<subprogram>> subprograms;
	std::vector<std::unique_ptr<subprogram>> new_subprograms;
	std::vector<const subprogram*> subprograms_to_stop;
	bool inside_run{ false };

private:

	virtual void update();

};

class fixed_time_step_loop : public loop {
public:

	fixed_time_step_loop();

	const fixed_frame_rate_controller& get_frame_rate_controller() const;

private:

	void update() override;

	fixed_frame_rate_controller frame_rate_controller;

};

}
