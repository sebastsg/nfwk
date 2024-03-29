#pragma once

#include "scripts/script_node.hpp"
#include "scripts/variables.hpp"
#include "script_node_macro.hpp"

namespace nfwk {

class modify_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(3, u8"Modify variable", u8"Variables");

	bool is_global{ false };
	other_variable_type other_type{ other_variable_type::value };
	std::u8string variable_name;
	std::u8string modify_value;
	variable_operator modify_operator{ variable_operator::set };

	script_node_output_type output_type() const override {
		return script_node_output_type::single;
	}

	std::optional<int> process() const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

}
