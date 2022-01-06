#pragma once

#include "scripts/script_node.hpp"
#include "script_node_macro.hpp"

namespace nfwk::script {

class execute_script_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(9, "Execute script", "");

	std::string script_id;

	output_type get_output_type() const override {
		return output_type::single;
	}

	std::optional<int> process(script_context& context) const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

}
