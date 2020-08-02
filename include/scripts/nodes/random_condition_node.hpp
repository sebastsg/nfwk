#pragma once

#include "scripts/script_node.hpp"

namespace no {

class random_condition_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(8, "Random true/false", "Random");

	int percent{ 50 };

	script_node_output_type output_type() const override {
		return script_node_output_type::boolean;
	}

	std::optional<int> process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

}
