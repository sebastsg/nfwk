#include "scripts/nodes/variable_exists_node.hpp"
#include "scripts/script_tree.hpp"
#include "scripts/variables.hpp"
#include "graphics/ui.hpp"

namespace nfwk {

std::optional<int> variable_exists_node::process() const {
	return tree->context.variables->find(scope_id, variable_name) ? 1 : 0;
}

void variable_exists_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read_bool();
	variable_name = stream.read_string();
}

void variable_exists_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write_bool(is_global);
	stream.write_string(variable_name);
}

}
