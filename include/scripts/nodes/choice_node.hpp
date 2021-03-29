#pragma once

#include "scripts/script_node.hpp"
#include "script_node_macro.hpp"

namespace nfwk {

class choice_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(1, u8"Choice", u8"");

	std::u8string text{ u8"Example text" };

	script_node_output_type output_type() const override {
		return script_node_output_type::single;
	}

	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool can_be_entry_point() const override;
	bool is_interactive() const override;

};

}
