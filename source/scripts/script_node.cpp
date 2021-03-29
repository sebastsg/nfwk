#include "scripts/script_node.hpp"
#include "scripts/script_tree.hpp"
#include "io.hpp"
#include "log.hpp"
#include "assert.hpp"
#include "utility_functions.hpp"

#include "scripts/nodes/message_node.hpp"
#include "scripts/nodes/choice_node.hpp"
#include "scripts/nodes/compare_variable_node.hpp"
#include "scripts/nodes/modify_variable_node.hpp"
#include "scripts/nodes/create_variable_node.hpp"
#include "scripts/nodes/variable_exists_node.hpp"
#include "scripts/nodes/delete_variable_node.hpp"
#include "scripts/nodes/random_output_node.hpp"
#include "scripts/nodes/random_condition_node.hpp"
#include "scripts/nodes/execute_script_node.hpp"
#include "scripts/nodes/trigger_event_node.hpp"

namespace nfwk {

// todo: move to own file
script_node_factory::script_node_factory() {
	register_node<message_node>();
	register_node<choice_node>();
	register_node<compare_variable_node>();
	register_node<modify_variable_node>();
	register_node<create_variable_node>();
	register_node<variable_exists_node>();
	register_node<delete_variable_node>();
	register_node<random_output_node>();
	register_node<random_condition_node>();
	register_node<execute_script_node>();
	register_node<trigger_event_node>();
}

const script_node_constructor* script_node_factory::find_constructor(int type) const {
	if (type >= 0xffff) {
		type -= 0xffff;
		if (static_cast<int>(user_nodes.size()) > type) {
			ASSERT(user_nodes[type].get_type() == type);
			return &user_nodes[type];
		}
	} else {
		if (static_cast<int>(core_nodes.size()) > type) {
			ASSERT(core_nodes[type].get_type() == type);
			return &core_nodes[type];
		}
	}
	return nullptr;
}

std::shared_ptr<script_node> script_node_factory::create_node(int type) const {
	return find_constructor(type)->construct();
}

void script_node_factory::register_node(int type, std::u8string_view name, std::u8string_view category, const std::function<std::shared_ptr<script_node>()>& constructor) {
	ASSERT(!find_constructor(type));
	if (type >= 0xffff) {
		user_nodes.resize(type + 1);
		user_nodes[type] = { type, name, category, constructor };
	} else {
		core_nodes.resize(type + 1);
		core_nodes[type] = { type, name, category, constructor };
	}
}

const std::vector<script_node_constructor>& script_node_factory::get_core_constructors() const {
	return core_nodes;
}

const std::vector<script_node_constructor>& script_node_factory::get_user_constructors() const {
	return user_nodes;
}

std::vector<script_node_constructor> script_node_factory::get_all_constructors() const {
	auto constructors = merge_vectors(get_core_constructors(), get_user_constructors());
	for (int i{ 0 }; i < static_cast<int>(constructors.size()); i++) {
		if (!constructors[i].is_valid()) {
			constructors.erase(constructors.begin() + i);
			i--;
		}
	}
	return constructors;
}

script_node_constructor::script_node_constructor(int type, std::u8string_view name, std::u8string_view category, const std::function<std::shared_ptr<script_node>()>& constructor)
	: type{ type }, name{ name }, category{ category }, constructor{ constructor } {

}

std::shared_ptr<script_node> script_node_constructor::construct() const {
	return constructor ? constructor() : nullptr;
}

std::optional<int> script_node_constructor::get_type() const {
	return type;
}

std::u8string_view script_node_constructor::get_name() const {
	return name;
}

std::u8string_view script_node_constructor::get_category() const {
	return category;
}

bool script_node_constructor::is_valid() const {
	return type.has_value() && constructor;
}

std::optional<int> script_node::process() const {
	return std::nullopt;
}

void script_node::write(io_stream& stream) const {
	stream.write<std::int32_t>(id);
	stream.write_optional<std::int32_t>(scope_id);
	stream.write(transform);
	stream.write_size(outputs.size());
	for (const auto& output : outputs) {
		stream.write<std::int32_t>(output.to_node());
		stream.write<std::int32_t>(output.slot());
	}
}

void script_node::read(io_stream& stream) {
	id = stream.read<std::int32_t>();
	scope_id = stream.read_optional<std::int32_t>();
	transform = stream.read<transform2>();
	const auto out_count = stream.read_size();
	for (int j{ 0 }; j < out_count; j++) {
		const auto node_id = stream.read<std::int32_t>();
		const auto out_id = stream.read<std::int32_t>();
		outputs.emplace_back(node_id, out_id);
	}
}

bool script_node::can_be_entry_point() const {
	return true;
}

bool script_node::is_interactive() const {
	return false;
}

void script_node::delete_output_node(int node_id) {
#ifdef NFWK_CPP_20
	std::erase_if(outputs, [node_id](const auto& output) {
		return output.to_node() == node_id;
	});
#else
	for (int i{ 0 }; i < used_output_slots_count(); i++) {
		if (outputs[i].to_node() == node_id) {
			outputs.erase(outputs.begin() + i);
			i--;
		}
	}
#endif
}

void script_node::delete_output_slot(int slot) {
#ifdef NFWK_CPP_20
	std::erase_if(outputs, [slot](const auto& output) {
		return output.slot() == slot;
	});
#else
	for (int i{ 0 }; i < used_output_slots_count(); i++) {
		if (outputs[i].slot() == slot) {
			outputs.erase(outputs.begin() + i);
			i--;
		}
	}
#endif
}

std::optional<int> script_node::get_output_node(int slot) const {
	for (const auto& output : outputs) {
		if (output.slot() == slot) {
			return output.to_node();
		}
	}
	return std::nullopt;
}

std::optional<script_node_output> script_node::get_first_output() const {
	return outputs.empty() ? std::optional<script_node_output>{} : outputs[0];
}

std::optional<int> script_node::get_first_output_node() const {
	if (const auto output = get_first_output()) {
		return output->to_node();
	} else {
		return std::nullopt;
	}
}

void script_node::add_output(std::optional<int> slot, int to_node_id) {
	if (!slot.has_value()) {
		slot = 0;
		while (get_output_node(slot.value())) {
			slot.value()++;
		}
	}
	for (auto& output : outputs) {
		if (output.slot() == slot.value()) {
			output.set_to_node(to_node_id);
			return;
		}
	}
	outputs.emplace_back(to_node_id, slot.value());
}

#if 0
bool script_node::has_interactive_output_nodes() const {
	return std::any_of(outputs.begin(), outputs.end(), [this](const auto& output) {
		return tree->get_node(output.to_node())->is_interactive();
	});
}
#endif

const std::vector<script_node_output>& script_node::get_outputs() const {
	return outputs;
}

int script_node::used_output_slots_count() const {
	return static_cast<int>(outputs.size());
}

}
