module;

#include "script_node_macro.hpp"

export module nfwk.scripts:node.message;

import std.core;
import nfwk.core;
import :node;

export namespace nfwk {

class message_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(0, "Message", "");

	std::string text{ "Example text" };

	script_node_output_type output_type() const override {
		return script_node_output_type::variable;
	}

	std::optional<int> process() const override {
		return std::nullopt;
	}

	void write(io_stream& stream) const override {
		script_node::write(stream);
		stream.write(text);
	}

	void read(io_stream& stream) override {
		script_node::read(stream);
		text = stream.read<std::string>();
	}

	bool is_interactive() const override {
		return true;
	}

};

}
