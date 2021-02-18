module;

#include "script_node_macro.hpp"

export module nfwk.scripts:node.variable_exists;

import std.core;
import nfwk.core;
import :node;
import :variables;

export namespace nfwk {

class variable_exists_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(5, "If variable exists", "Variables");

	bool is_global{ false };
	std::string variable_name;

	script_node_output_type output_type() const override {
		return script_node_output_type::boolean;
	}

	std::optional<int> process() const override {
		return find_variable(scope_id, variable_name) ? 1 : 0;
	}

	void write(io_stream& stream) const override {
		script_node::write(stream);
		stream.write<bool>(is_global);
		stream.write(variable_name);
	}

	void read(io_stream& stream) override {
		script_node::read(stream);
		is_global = stream.read<bool>();
		variable_name = stream.read<std::string>();
	}

};

}
