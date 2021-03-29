#include "scripts/nodes/modify_variable_node.hpp"
#include "scripts/script_tree.hpp"
#include "graphics/ui.hpp"
#include "log.hpp"

namespace nfwk {

std::optional<int> modify_variable_node::process() const {
	if (modify_value == u8"") {
		return 0;
	}
	auto context = tree->context;
	auto variable = context.variables->find(scope_id, variable_name);
	if (!variable) {
		warning(scripts::log, u8"Attempted to modify {} (global: {}) but it does not exist", variable_name, is_global);
		return 0;
	}
	if (other_type == other_variable_type::local) {
		if (const auto* local_variable = context.variables->find(scope_id, modify_value)) {
			variable->modify(*local_variable, modify_operator);
		} else {
			warning(scripts::log, u8"Cannot modify {} because the local variable {} does not exist.", variable_name, modify_value);
			return 0;
		}
	} else if (other_type == other_variable_type::global) {
		if (const auto* global_variable = context.variables->find(std::nullopt, modify_value)) {
			variable->modify(*global_variable, modify_operator);
		} else {
			warning(scripts::log, u8"Cannot modify {} because the global variable {} does not exist.", variable_name, modify_value);
			return 0;
		}
	}
	return 0;
}

void modify_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write_bool(is_global);
	stream.write(static_cast<std::int32_t>(other_type));
	stream.write_string(variable_name);
	stream.write_string(modify_value);
	stream.write(static_cast<std::int32_t>(modify_operator));
}

void modify_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read_bool();
	other_type = static_cast<other_variable_type>(stream.read<std::int32_t>());
	variable_name = stream.read_string();
	modify_value = stream.read_string();
	modify_operator = static_cast<variable_operator>(stream.read<std::int32_t>());
}

}
