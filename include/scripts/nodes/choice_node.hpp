#pragma once

#include "scripts/script_node.hpp"
#include "script_node_macro.hpp"

namespace nfwk::script {

class choice_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(1, "Choice", "");

	std::string text{ "Example text" };

	output_type get_output_type() const override {
		return output_type::single;
	}

	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool can_be_entry_point() const override;
	bool is_interactive() const override;

};

}
