module;

#include "script_node_macro.hpp"

export module nfwk.scripts:node.compare_variable;

import std.core;
import nfwk.core;
import :node;
import :variables;

export namespace nfwk {

class compare_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(2, "Compare variable", "Variables");

	bool is_global{ false };
	other_variable_type other_type{ other_variable_type::value };
	std::string variable_name;
	std::string comparison_value;
	variable_comparison comparison_operator{ variable_comparison::equal };

	script_node_output_type output_type() const override {
		return script_node_output_type::boolean;
	}

	std::optional<int> process() const override {
		const auto variable = find_variable(scope_id, variable_name);
		if (!variable) {
			warning("scripts", "Attempted to check {} (global: {}) but it does not exist", variable_name, is_global);
			return std::nullopt;
		}
		if (other_type == other_variable_type::local) {
			if (const auto local_variable = find_variable(scope_id, comparison_value)) {
				return variable->get().compare(local_variable->get(), comparison_operator) ? 1 : 0;
			} else {
				warning("scripts", "Cannot compare against {} because local variable {} does not exist.", variable_name, comparison_value);
			}
		} else if (other_type == other_variable_type::global) {
			if (const auto global_variable = find_variable(std::nullopt, comparison_value)) {
				return variable->get().compare(global_variable->get(), comparison_operator) ? 1 : 0;
			} else {
				warning("scripts", "Cannot compare against {} because global variable {} does not exist.", variable_name, comparison_value);
			}
		} else if (other_type == other_variable_type::value) {
			switch (variable->get().type) {
			case variable_type::string: return variable->get().compare(comparison_value, comparison_operator);
			case variable_type::integer:
			case variable_type::boolean: return variable->get().compare(std::stoi(comparison_value), comparison_operator);
			case variable_type::floating: return variable->get().compare(std::stof(comparison_value), comparison_operator);
			}
		}
		return 0;
	}

	void write(io_stream& stream) const override {
		script_node::write(stream);
		stream.write<std::uint8_t>(is_global);
		stream.write<std::int32_t>(static_cast<std::int32_t>(other_type));
		stream.write(variable_name);
		stream.write(comparison_value);
		stream.write<std::int32_t>(static_cast<std::int32_t>(comparison_operator));
	}

	void read(io_stream& stream) override {
		script_node::read(stream);
		is_global = (stream.read<std::uint8_t>() != 0);
		other_type = static_cast<other_variable_type>(stream.read<std::int32_t>());
		variable_name = stream.read<std::string>();
		comparison_value = stream.read<std::string>();
		comparison_operator = static_cast<variable_comparison>(stream.read<std::int32_t>());
	}

};

}
