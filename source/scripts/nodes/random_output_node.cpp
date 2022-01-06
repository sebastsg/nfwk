#include "scripts/nodes/random_output_node.hpp"
#include "random.hpp"
#include "log.hpp"

namespace nfwk::script {

std::optional<int> random_output_node::process(script_context& context) const {
	if (outputs.empty()) {
		warning(scripts::log, "No nodes attached.");
		return std::nullopt;
	} else {
		return random_number_generator::any().next(static_cast<int>(outputs.size()) - 1);
	}
}

void random_output_node::write(io_stream& stream) const {
	script_node::write(stream);
}

void random_output_node::read(io_stream& stream) {
	script_node::read(stream);
}

}
