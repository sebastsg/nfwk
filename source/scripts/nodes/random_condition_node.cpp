#include "scripts/nodes/random_condition_node.hpp"
#include "random.hpp"
#include "io.hpp"

namespace nfwk::script {

std::optional<int> random_condition_node::process(script_context& context) const {
	return random_number_generator::any().chance(chance);
}

void random_condition_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write_float32(chance);
}

void random_condition_node::read(io_stream& stream) {
	script_node::read(stream);
	chance = stream.read_float32();
}

}
