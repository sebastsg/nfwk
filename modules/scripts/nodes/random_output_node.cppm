module;

#include "script_node_macro.hpp"

export module nfwk.scripts:node.random_output;

import std.core;
import nfwk.core;
import nfwk.random;
import :node;

export namespace nfwk {

class random_output_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(7, "Random output", "Random");

	script_node_output_type output_type() const override {
		return script_node_output_type::variable;
	}

	std::optional<int> process() const override {
		if (outputs.empty()) {
			warning("scripts", "No nodes attached.");
			return std::nullopt;
		} else {
			return random_number_generator::global().next(static_cast<int>(outputs.size()) - 1);
		}
	}

	void write(io_stream& stream) const override {
		script_node::write(stream);
	}

	void read(io_stream& stream) override {
		script_node::read(stream);
	}

};

}
