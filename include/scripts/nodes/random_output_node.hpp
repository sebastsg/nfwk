#pragma once

#include "scripts/script_node.hpp"
#include "script_node_macro.hpp"

namespace nfwk {

class random_output_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(7, u8"Random output", u8"Random");

	script_node_output_type output_type() const override {
		return script_node_output_type::variable;
	}

	std::optional<int> process() const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

}
