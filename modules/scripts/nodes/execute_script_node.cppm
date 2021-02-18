module;

#include "script_node_macro.hpp"

export module nfwk.scripts:node.execute_script;

import std.core;
import nfwk.core;
import :node;

export namespace nfwk {

class execute_script_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(9, "Execute script", "");

	std::string script_id;

	script_node_output_type output_type() const override {
		return script_node_output_type::single;
	}

	std::optional<int> process() const override {
		/*if (auto script = run_script(script_id)) {
			done_event = script->events.done.listen([this] {
				process_output(0);
			});
			return std::nullopt;
		} else {
			done_event.stop();
			return 0;
		}*/
		return std::nullopt;
	}

	void write(io_stream& stream) const override {
		script_node::write(stream);
		stream.write(script_id);
	}

	void read(io_stream& stream) override {
		script_node::read(stream);
		script_id = stream.read<std::string>();
	}

	bool is_interactive() const override {
		return true;
	}

private:

	mutable event_listener done_event;

};

}
