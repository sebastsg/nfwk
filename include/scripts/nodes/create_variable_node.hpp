#pragma once

#include "scripts/script_node.hpp"
#include "scripts/variables.hpp"

namespace no {

class create_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(4, "Create variable", "Variables");

	variable new_variable;
	bool is_global{ false };
	bool overwrite{ false };

	script_node_output_type output_type() const override {
		return script_node_output_type::single;
	}

	std::optional<int> process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

}
