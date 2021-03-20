#include "scripts/nodes/random_output_node.hpp"
#include "random.hpp"
#include "log.hpp"

namespace nfwk {

std::optional<int> random_output_node::process() {
	if (outputs.empty()) {
		warning("scripts", "No nodes attached.");
		return std::nullopt;
	} else {
		return random_number_generator::global().next(static_cast<int>(outputs.size()) - 1);
	}
}

void random_output_node::write(io_stream& stream) const {
	script_node::write(stream);
}

void random_output_node::read(io_stream& stream) {
	script_node::read(stream);
}

bool random_output_node::update_editor() {
	return false;
}

}
