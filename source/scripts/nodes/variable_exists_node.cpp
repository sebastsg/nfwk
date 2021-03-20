#include "scripts/nodes/variable_exists_node.hpp"
#include "scripts/script_tree.hpp"
#include "scripts/variables.hpp"
#include "graphics/ui.hpp"

namespace nfwk {

std::optional<int> variable_exists_node::process() {
	return tree->context->find(scope_id, variable_name) ? 1 : 0;
}

void variable_exists_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read<bool>();
	variable_name = stream.read<std::string>();
}

void variable_exists_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<bool>(is_global);
	stream.write(variable_name);
}

bool variable_exists_node::update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::input("Name", variable_name);
	return dirty;
}

}
