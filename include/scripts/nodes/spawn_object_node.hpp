#pragma once

#include "io.hpp"
#include "scripts/script_node.hpp"
#include "script_node_macro.hpp"
#include "scripts/script_tree.hpp"

namespace nfwk::script {

class spawn_object_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(11, "Spawn object", "");

	std::string class_id{ 0 };
	vector2f position;

	output_type get_output_type() const override {
		return output_type::single;
	}

	std::optional<int> process(script_context& context) const override {
		if (auto class_ = context.objects->find_class_instance(class_id)) {
			class_->create_instance();
		}
		return 0;
	}

	void write(io_stream& stream) const override {
		script_node::write(stream);
		stream.write_string(class_id);
		stream.write_struct(position);
	}

	void read(io_stream& stream) override {
		script_node::read(stream);
		class_id = stream.read_string();
		position = stream.read_struct<vector2f>();
	}

};

}
