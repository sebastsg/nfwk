module;

#include "script_node_macro.hpp"

export module nfwk.scripts:node.delete_variable;

import std.core;
import :node;
import :variables;

export namespace nfwk {

class delete_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(6, "Delete variable", "Variables");

	bool is_global{ false };
	std::string variable_name;

	script_node_output_type output_type() const override {
		return script_node_output_type::single;
	}

	std::optional<int> process() const override {
		delete_variable(scope_id, variable_name);
		return 0;
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
