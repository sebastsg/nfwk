#pragma once

#include "scripts/script_node.hpp"

namespace no {

class message_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(0, "Message", "");

	std::string text{ "Example text" };

	script_node_output_type output_type() const override {
		return script_node_output_type::variable;
	}

	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;
	bool is_interactive() const override;

};

}
