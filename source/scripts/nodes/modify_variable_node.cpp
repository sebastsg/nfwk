#include "scripts/nodes/modify_variable_node.hpp"
#include "scripts/script_tree.hpp"
#include "graphics/ui.hpp"
#include "log.hpp"

namespace nfwk {

std::optional<int> modify_variable_node::process() {
	if (modify_value == "") {
		return 0;
	}
	auto context = tree->context;
	auto variable = context->find(scope_id, variable_name);
	if (!variable) {
		warning("scripts", "Attempted to modify {} (global: {}) but it does not exist", variable_name, is_global);
		return 0;
	}
	if (other_type == other_variable_type::local) {
		if (const auto local_variable = context->find(scope_id, modify_value)) {
			variable->get().modify(local_variable->get(), modify_operator);
		} else {
			warning("scripts", "Cannot modify {} because the local variable {} does not exist.", variable_name, modify_value);
			return 0;
		}
	} else if (other_type == other_variable_type::global) {
		if (const auto global_variable = context->find(std::nullopt, modify_value)) {
			variable->get().modify(global_variable->get(), modify_operator);
		} else {
			warning("scripts", "Cannot modify {} because the global variable {} does not exist.", variable_name, modify_value);
			return 0;
		}
	}
	return 0;
}

void modify_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write(is_global);
	stream.write(static_cast<std::int32_t>(other_type));
	stream.write(variable_name);
	stream.write(modify_value);
	stream.write(static_cast<std::int32_t>(modify_operator));
}

void modify_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read<bool>();
	other_type = static_cast<other_variable_type>(stream.read<std::int32_t>());
	variable_name = stream.read<std::string>();
	modify_value = stream.read<std::string>();
	modify_operator = static_cast<variable_operator>(stream.read<std::int32_t>());
}

bool modify_variable_node::update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::input("Name", variable_name);
	if (auto new_operator = ui::combo("Operator", { "Set", "Negate", "Add", "Multiply", "Divide" }, static_cast<int>(modify_operator))) {
		modify_operator = static_cast<variable_operator>(new_operator.value());
		dirty = true;
	}
	if (auto new_type = ui::combo("##other-type", { "Value", "Local", "Global" }, static_cast<int>(other_type))) {
		other_type = static_cast<other_variable_type>(new_type.value());
		dirty = true;
	}
	dirty |= ui::input("##set-value", modify_value);
	return dirty;
}

}
