#pragma once

#include "scripts/script_node.hpp"
#include "scripts/variables.hpp"
#include "script_node_macro.hpp"

namespace nfwk {

class compare_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(2, u8"Compare variable", u8"Variables");

	bool is_global{ false };
	other_variable_type other_type{ other_variable_type::value };
	std::u8string variable_name;
	std::u8string comparison_value;
	variable_comparison comparison_operator{ variable_comparison::equal };

	script_node_output_type output_type() const override {
		return script_node_output_type::boolean;
	}

	std::optional<int> process() const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

}
