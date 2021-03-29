#pragma once

#include "scripts/script_node.hpp"
#include "script_node_macro.hpp"

namespace nfwk {

class delete_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(6, u8"Delete variable", u8"Variables");

	bool is_global{ false };
	std::u8string variable_name;

	script_node_output_type output_type() const override {
		return script_node_output_type::single;
	}

	std::optional<int> process() const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

}
