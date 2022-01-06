#include "scripts/nodes/variable_nodes.hpp"
#include "scripts/script_tree.hpp"
#include "log.hpp"

namespace nfwk::script {

std::optional<int> compare_variable_node::process(script_context& context) const {
	const auto* variable = context.variables->find(context.scope_id, variable_name);
	if (!variable) {
		warning(scripts::log, "Attempted to check {} (global: {}) but it does not exist", variable_name, is_global);
		return std::nullopt;
	}
	std::string value{ comparison_value };
	if (other_type == other_variable_type::local) {
		if (const auto* local_variable = context.variables->find(context.scope_id, comparison_value)) {
			return variable->compare(*local_variable, comparison_operator) ? 1 : 0;
		} else {
			warning(scripts::log, "Cannot compare against {} because local variable {} does not exist.", variable_name, comparison_value);
		}
	} else if (other_type == other_variable_type::global) {
		if (const auto* global_variable = context.variables->find(std::nullopt, comparison_value)) {
			return variable->compare(*global_variable, comparison_operator) ? 1 : 0;
		} else {
			warning(scripts::log, "Cannot compare against {} because global variable {} does not exist.", variable_name, comparison_value);
		}
	}
	return 0;
}

void compare_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write_bool(is_global);
	stream.write_int32(other_type);
	stream.write_string(variable_name);
	stream.write_string(comparison_value);
	stream.write_int32(comparison_operator);
}

void compare_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read_bool();
	other_type = stream.read_int32<other_variable_type>();
	variable_name = stream.read_string();
	comparison_value = stream.read_string();
	comparison_operator = stream.read_int32<variable_comparison>();
}

std::optional<int> modify_variable_node::process(script_context& context) const {
	if (modify_value.empty()) {
		return 0;
	}
	auto variable = context.variables->find(context.scope_id, variable_name);
	if (!variable) {
		warning(scripts::log, "Attempted to modify {} (global: {}) but it does not exist", variable_name, is_global);
		return 0;
	}
	if (other_type == other_variable_type::local) {
		if (const auto* local_variable = context.variables->find(context.scope_id, modify_value)) {
			variable->modify(*local_variable, modify_operator);
		} else {
			warning(scripts::log, "Cannot modify {} because the local variable {} does not exist.", variable_name, modify_value);
			return 0;
		}
	} else if (other_type == other_variable_type::global) {
		if (const auto* global_variable = context.variables->find(std::nullopt, modify_value)) {
			variable->modify(*global_variable, modify_operator);
		} else {
			warning(scripts::log, "Cannot modify {} because the global variable {} does not exist.", variable_name, modify_value);
			return 0;
		}
	}
	return 0;
}

void modify_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write_bool(is_global);
	stream.write_int32(other_type);
	stream.write_string(variable_name);
	stream.write_string(modify_value);
	stream.write_int32(modify_operator);
}

void modify_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read_bool();
	other_type = stream.read_int32<other_variable_type>();
	variable_name = stream.read_string();
	modify_value = stream.read_string();
	modify_operator = stream.read_int32<variable_operator>();
}

std::optional<int> create_variable_node::process(script_context& context) const {
	if (auto existing_variable = context.variables->find(context.scope_id, new_variable.name)) {
		if (overwrite) {
			*existing_variable = new_variable;
		}
	} else {
		context.variables->add(context.scope_id, new_variable);
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
	const auto a = sizeof(variable);
}

std::optional<int> variable_exists_node::process(script_context& context) const {
	return context.variables->find(context.scope_id, variable_name) ? 1 : 0;
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

std::optional<int> delete_variable_node::process(script_context& context) const {
	context.variables->remove(context.scope_id, variable_name);
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
