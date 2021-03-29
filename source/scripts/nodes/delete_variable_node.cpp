#include "scripts/nodes/delete_variable_node.hpp"
#include "scripts/script_tree.hpp"
#include "scripts/variables.hpp"
#include "graphics/ui.hpp"

namespace nfwk {

std::optional<int> delete_variable_node::process() const {
	tree->context.variables->remove(scope_id, variable_name);
	return 0;
}

void delete_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write_bool(is_global);
	stream.write_string(variable_name);
}

void delete_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read_bool();
	variable_name = stream.read_string();
}

}
