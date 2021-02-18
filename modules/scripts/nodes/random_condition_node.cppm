module;

#include "script_node_macro.hpp"

export module nfwk.scripts:node.random_condition;

import std.core;
import nfwk.core;
import nfwk.random;
import :node;

export namespace nfwk {

class random_condition_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(8, "Random true/false", "Random");

	int percent{ 50 };

	script_node_output_type output_type() const override {
		return script_node_output_type::boolean;
	}

	std::optional<int> process() const override {
		return random_number_generator::global().chance(0.5f);
	}

	void write(io_stream& stream) const override {
		script_node::write(stream);
		stream.write<std::int32_t>(percent);
	}

	void read(io_stream& stream) override {
		script_node::read(stream);
		percent = stream.read<std::int32_t>();
	}

};

}
