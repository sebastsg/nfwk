#pragma once

#include "scripts/script_node.hpp"
#include "script_node_macro.hpp"

namespace nfwk::script {

class message_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(0, "Message", "");

	std::string text{ "Example text" };

	output_type get_output_type() const override {
		return output_type::variable;
	}

	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool is_interactive() const override;

};

}
