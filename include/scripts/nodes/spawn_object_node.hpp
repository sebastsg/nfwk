#pragma once

#include "io.hpp"
#include "scripts/script_node.hpp"
#include "script_node_macro.hpp"

namespace nfwk {

class spawn_object_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(11, u8"Spawn object", u8"");

	int class_index{ 0 };
	vector2f position;

	script_node_output_type output_type() const override {
		return script_node_output_type::single;
	}

	std::optional<int> process() const override {
		objects::spawn(class_index);
		return 0;
	}

	void write(io_stream& stream) const override {
		script_node::write(stream);
		stream.write(static_cast<std::int32_t>(class_index));
		stream.write(position);
	}

	void read(io_stream& stream) override {
		script_node::read(stream);
		class_index = stream.read<std::int32_t>();
		position = stream.read<vector2f>();
	}

};

}
