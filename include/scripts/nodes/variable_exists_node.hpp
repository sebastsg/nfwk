#pragma once

#include "scripts/script_node.hpp"
#include "script_node_macro.hpp"

namespace nfwk {

class variable_exists_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(5, "If variable exists", "Variables");

	bool is_global{ false };
	std::string variable_name;

	script_node_output_type output_type() const override {
		return script_node_output_type::boolean;
	}

	std::optional<int> process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

}
