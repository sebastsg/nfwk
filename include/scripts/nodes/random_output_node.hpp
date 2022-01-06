#pragma once

#include "scripts/script_node.hpp"
#include "script_node_macro.hpp"

namespace nfwk::script {

class random_output_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(7, "Random output", "Random");

	output_type get_output_type() const override {
		return output_type::variable;
	}

	std::optional<int> process(script_context& context) const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

}
