#include "subprogram.hpp"
#include "loop.hpp"

namespace nfwk {

subprogram::subprogram(loop& loop) : owning_loop{ &loop } {
	info("core", "Created subprogram: {}", typeid(*this).name());
}

subprogram::~subprogram() {
	if (has_next_state()) {
		owning_loop->add(make_next_subprogram());
	}
}

void subprogram::stop() {
	owning_loop->remove(*this);
}

bool subprogram::has_next_state() const {
	return static_cast<bool>(make_next_subprogram);
}

void subprogram::change(const std::function<std::unique_ptr<subprogram>()>& make_subprogram) {
	make_next_subprogram = make_subprogram;
	stop();
}

loop& subprogram::get_loop() {
	return *owning_loop;
}

float subprogram::delta() const {
	return owning_loop->delta();
}

}
