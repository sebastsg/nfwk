module;

#include "script_node_macro.hpp"

export module nfwk.scripts:node.modify_variable;

import std.core;
import nfwk.core;
import :node;
import :variables;

export namespace nfwk {

class modify_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(3, "Modify variable", "Variables");

	bool is_global{ false };
	other_variable_type other_type{ other_variable_type::value };
	std::string variable_name;
	std::string modify_value;
	variable_operator modify_operator{ variable_operator::set };

	script_node_output_type output_type() const override {
		return script_node_output_type::single;
	}

	std::optional<int> process() const override {
		if (modify_value == "") {
			return 0;
		}
		auto variable = find_variable(scope_id, variable_name);
		if (!variable) {
			warning("scripts", "Attempted to modify {} (global: {}) but it does not exist", variable_name, is_global);
			return 0;
		}
		if (other_type == other_variable_type::local) {
			if (const auto local_variable = find_variable(scope_id, modify_value)) {
				variable->get().modify(local_variable->get(), modify_operator);
			} else {
				warning("scripts", "Cannot modify {} because the local variable {} does not exist.", variable_name, modify_value);
				return 0;
			}
		} else if (other_type == other_variable_type::global) {
			if (const auto global_variable = find_variable(std::nullopt, modify_value)) {
				variable->get().modify(global_variable->get(), modify_operator);
			} else {
				warning("scripts", "Cannot modify {} because the global variable {} does not exist.", variable_name, modify_value);
				return 0;
			}
		}
		return 0;
	}

	void write(io_stream& stream) const override {
		script_node::write(stream);
		stream.write<bool>(is_global);
		stream.write(static_cast<std::int32_t>(other_type));
		stream.write(variable_name);
		stream.write(modify_value);
		stream.write(static_cast<std::int32_t>(modify_operator));
	}

	void read(io_stream& stream) override {
		script_node::read(stream);
		is_global = stream.read<bool>();
		other_type = static_cast<other_variable_type>(stream.read<std::int32_t>());
		variable_name = stream.read<std::string>();
		modify_value = stream.read<std::string>();
		modify_operator = static_cast<variable_operator>(stream.read<std::int32_t>());
	}

};

}
