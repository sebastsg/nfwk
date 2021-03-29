#include "scripts/nodes/random_condition_node.hpp"
#include "graphics/ui.hpp"
#include "random.hpp"
#include "math.hpp"
#include "io.hpp"

namespace nfwk {

std::optional<int> random_condition_node::process() const {
	return random_number_generator::global().chance(0.5f);
}

void random_condition_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<std::int32_t>(percent);
}

void random_condition_node::read(io_stream& stream) {
	script_node::read(stream);
	percent = stream.read<std::int32_t>();
}

}
