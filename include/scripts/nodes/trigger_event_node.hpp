#pragma once

#include "scripts/script_node.hpp"
#include "script_node_macro.hpp"

namespace nfwk::script {

class trigger_event_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(10, "Trigger event", "");

	std::string event_id;

	output_type get_output_type() const override {
		return output_type::single;
	}

	std::optional<int> process(script_context& context) const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

}
