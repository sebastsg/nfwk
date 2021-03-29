#pragma once

#include "scripts/script_node.hpp"
#include "script_node_macro.hpp"

namespace nfwk {

class trigger_event_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(10, u8"Trigger event", u8"");

	std::u8string event_id;

	script_node_output_type output_type() const override {
		return script_node_output_type::single;
	}

	std::optional<int> process() const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

}
