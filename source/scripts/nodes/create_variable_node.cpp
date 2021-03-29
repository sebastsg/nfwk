#include "scripts/nodes/create_variable_node.hpp"
#include "scripts/script_tree.hpp"
#include "graphics/ui.hpp"

namespace nfwk {

std::optional<int> create_variable_node::process() const {
	auto context = tree->context;
	if (auto existing_variable = context.variables->find(scope_id, new_variable.name)) {
		if (overwrite) {
			*existing_variable = new_variable;
		}
	} else {
		context.variables->add(scope_id, new_variable);
	}
	return 0;
}

void create_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write_bool(is_global);
	stream.write_bool(overwrite);
	new_variable.write(stream);
}

void create_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read_bool();
	overwrite = stream.read_bool();
	new_variable.read(stream);
}

}
