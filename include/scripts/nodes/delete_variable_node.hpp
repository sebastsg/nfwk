#pragma once

#include "scripts/script_node.hpp"

namespace no {

class delete_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(6, "Delete variable", "Variables");

	bool is_global{ false };
	std::string variable_name;

	script_node_output_type output_type() const override {
		return script_node_output_type::single;
	}

	std::optional<int> process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

}
