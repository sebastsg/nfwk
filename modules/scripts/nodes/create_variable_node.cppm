module;

#include "script_node_macro.hpp"

export module nfwk.scripts:node.create_variable;

import std.core;
import :node;
import :variables;

export namespace nfwk {

class create_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(4, "Create variable", "Variables");

	variable new_variable;
	bool is_global{ false };
	bool overwrite{ false };

	script_node_output_type output_type() const override {
		return script_node_output_type::single;
	}

	std::optional<int> process() const override {
		create_variable(scope_id, new_variable, overwrite);
		return 0;
	}

	void write(io_stream& stream) const override {
		script_node::write(stream);
		stream.write<std::uint8_t>(is_global);
		stream.write<std::uint8_t>(overwrite);
		stream.write(static_cast<std::int32_t>(new_variable.type));
		stream.write(new_variable.name);
		switch (new_variable.type) {
		case variable_type::string:
			stream.write(std::get<std::string>(new_variable.value));
			break;
		case variable_type::integer:
			stream.write(static_cast<std::int32_t>(std::get<int>(new_variable.value)));
			break;
		case variable_type::boolean:
			stream.write<bool>(std::get<int>(new_variable.value) != 0);
			break;
		case variable_type::floating:
			stream.write(std::get<float>(new_variable.value));
			break;
		}
		stream.write<std::uint8_t>(new_variable.persistent);
	}

	void read(io_stream& stream) override {
		script_node::read(stream);
		is_global = stream.read<bool>();
		overwrite = stream.read<bool>();
		new_variable.type = static_cast<variable_type>(stream.read<std::int32_t>());
		new_variable.name = stream.read<std::string>();
		switch (new_variable.type) {
		case variable_type::string:
			new_variable.value = stream.read<std::string>();
			break;
		case variable_type::integer:
			new_variable.value = stream.read<std::int32_t>();
			break;
		case variable_type::boolean:
			new_variable.value = static_cast<int>(stream.read<bool>() ? 1 : 0);
			break;
		case variable_type::floating:
			new_variable.value = stream.read<float>();
			break;
		}
		new_variable.persistent = stream.read<bool>();
	}

};

}
