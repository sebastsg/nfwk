#include "scripts/nodes/create_variable_node.hpp"
#include "scripts/script_tree.hpp"
#include "graphics/ui.hpp"

namespace nfwk {

std::optional<int> create_variable_node::process() {
	auto context = tree->context;
	if (auto existing_variable = context->find(scope_id, new_variable.name)) {
		if (overwrite) {
			*existing_variable = new_variable;
		}
	} else {
		context->add(scope_id, new_variable);
	}
	return 0;
}

void create_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write(is_global);
	stream.write(overwrite);
	stream.write(static_cast<std::int32_t>(new_variable.type));
	stream.write(new_variable.name);
	stream.write(new_variable.value);
	stream.write(new_variable.persistent);
}

void create_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read<bool>();
	overwrite = stream.read<bool>();
	new_variable.type = static_cast<variable_type>(stream.read<std::int32_t>());
	new_variable.name = stream.read<std::string>();
	new_variable.value = stream.read<std::string>();
	new_variable.persistent = stream.read<bool>();
}

bool create_variable_node::update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::checkbox("Persistent", new_variable.persistent);
	dirty |= ui::checkbox("Overwrite", overwrite);
	if (auto new_type = ui::combo("Type", { "String", "Integer", "Boolean", "Float" }, static_cast<int>(new_variable.type))) {
		new_variable.type = static_cast<variable_type>(new_type.value());
		dirty = true;
	}
	dirty |= ui::input("Name", new_variable.name);
	if (new_variable.type == variable_type::string) {
		dirty |= ui::input("Value##string", std::get<std::string>(new_variable.value));
	} else if (new_variable.type == variable_type::integer) {
		dirty |= ui::input("Value##integer", std::get<int>(new_variable.value));
	} else if (new_variable.type == variable_type::floating) {
		dirty |= ui::input("Value##float", std::get<float>(new_variable.value));
	} else if (new_variable.type == variable_type::boolean) {
		bool as_bool = (std::get<int>(new_variable.value) != 0);
		dirty |= ui::checkbox("Value##bool", as_bool);
		new_variable.value = as_bool ? 1 : 0;
	}
	return dirty;
}

}
