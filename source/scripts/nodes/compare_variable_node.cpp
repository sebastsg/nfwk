#include "scripts/nodes/compare_variable_node.hpp"
#include "scripts/script_tree.hpp"
#include "graphics/ui.hpp"
#include "debug.hpp"

namespace no {

std::optional<int> compare_variable_node::process() {
	auto context = tree->context;
	const auto variable = context->find(scope_id, variable_name);
	if (!variable) {
		WARNING_X("scripts", "Attempted to check " << variable_name << " (global: " << is_global << ") but it does not exist");
		return std::nullopt;
	}
	std::string value{ comparison_value };
	if (other_type == other_variable_type::local) {
		if (const auto local_variable = context->find(scope_id, comparison_value)) {
			return variable->get().compare(local_variable->get(), comparison_operator) ? 1 : 0;
		} else {
			WARNING_X("scripts", "Cannot compare against " << variable_name << " because local variable " << comparison_value << " does not exist.");
		}
	} else if (other_type == other_variable_type::global) {
		if (const auto global_variable = context->find(std::nullopt, comparison_value)) {
			return variable->get().compare(global_variable->get(), comparison_operator) ? 1 : 0;
		} else {
			WARNING_X("scripts", "Cannot compare against " << variable_name << " because global variable " << comparison_value << " does not exist.");
		}
	}
	return 0;
}

void compare_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write<int32_t>(static_cast<int32_t>(other_type));
	stream.write(variable_name);
	stream.write(comparison_value);
	stream.write<int32_t>(static_cast<int32_t>(comparison_operator));
}

void compare_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = (stream.read<uint8_t>() != 0);
	other_type = static_cast<other_variable_type>(stream.read<int32_t>());
	variable_name = stream.read<std::string>();
	comparison_value = stream.read<std::string>();
	comparison_operator = static_cast<variable_comparison>(stream.read<int32_t>());
}

bool compare_variable_node::update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::input("Name", variable_name);
	if (auto new_comparison = ui::combo("Comparison", { "==", "!=", ">", "<", ">=", "<=" }, static_cast<int>(comparison_operator))) {
		comparison_operator = static_cast<variable_comparison>(new_comparison.value());
		dirty = true;
	}
	if (auto new_type = ui::combo("##other-type", { "Value", "Local", "Global" }, static_cast<int>(other_type))) {
		other_type = static_cast<other_variable_type>(new_type.value());
		dirty = true;
	}
	dirty |= ui::input("Value", comparison_value);
	return dirty;
}

}
