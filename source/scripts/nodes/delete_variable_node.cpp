#include "scripts/nodes/delete_variable_node.hpp"
#include "scripts/script_tree.hpp"
#include "scripts/variables.hpp"
#include "graphics/ui.hpp"

namespace no {

std::optional<int> delete_variable_node::process() {
	tree->context->remove(scope_id, variable_name);
	return 0;
}

void delete_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<bool>(is_global);
	stream.write(variable_name);
}

void delete_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read<bool>();
	variable_name = stream.read<std::string>();
}

bool delete_variable_node::update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::input("Name", variable_name);
	return dirty;
}

}
