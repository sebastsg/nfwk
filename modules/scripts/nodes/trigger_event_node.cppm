module;

#include "script_node_macro.hpp"

export module nfwk.scripts:node.trigger_event;

import std.core;
import nfwk.core;
import :node;
import :events;

export namespace nfwk {

class trigger_event_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(10, "Trigger event", "");

	std::string event_id;

	script_node_output_type output_type() const override {
		return script_node_output_type::single;
	}

	std::optional<int> process() const override {
		warning("scripts", "Does not pause script yet. Should be made like execute_script_node.");
		game_event_container::global().trigger(event_id);
		return 0;
	}

	void write(io_stream& stream) const override {
		script_node::write(stream);
		stream.write(event_id);
	}

	void read(io_stream& stream) override {
		script_node::read(stream);
		event_id = stream.read<std::string>();
	}

};

}
