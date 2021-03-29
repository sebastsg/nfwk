#include "scripts/nodes/compare_variable_node.hpp"
#include "scripts/script_tree.hpp"
#include "graphics/ui.hpp"
#include "log.hpp"

namespace nfwk {

std::optional<int> compare_variable_node::process() const {
	auto context = tree->context;
	const auto* variable = context.variables->find(scope_id, variable_name);
	if (!variable) {
		warning(scripts::log, u8"Attempted to check {} (global: {}) but it does not exist", variable_name, is_global);
		return std::nullopt;
	}
	std::u8string value{ comparison_value };
	if (other_type == other_variable_type::local) {
		if (const auto* local_variable = context.variables->find(scope_id, comparison_value)) {
			return variable->compare(*local_variable, comparison_operator) ? 1 : 0;
		} else {
			warning(scripts::log, u8"Cannot compare against {} because local variable {} does not exist.", variable_name, comparison_value);
		}
	} else if (other_type == other_variable_type::global) {
		if (const auto* global_variable = context.variables->find(std::nullopt, comparison_value)) {
			return variable->compare(*global_variable, comparison_operator) ? 1 : 0;
		} else {
			warning(scripts::log, u8"Cannot compare against {} because global variable {} does not exist.", variable_name, comparison_value);
		}
	}
	return 0;
}

void compare_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write_bool(is_global);
	stream.write<std::int32_t>(static_cast<std::int32_t>(other_type));
	stream.write_string(variable_name);
	stream.write_string(comparison_value);
	stream.write<std::int32_t>(static_cast<std::int32_t>(comparison_operator));
}

void compare_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read_bool();
	other_type = static_cast<other_variable_type>(stream.read<std::int32_t>());
	variable_name = stream.read_string();
	comparison_value = stream.read_string();
	comparison_operator = static_cast<variable_comparison>(stream.read<std::int32_t>());
}

}
