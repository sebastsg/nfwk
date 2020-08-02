#include "scripts/script_node.hpp"
#include "scripts/script_tree.hpp"
#include "io.hpp"
#include "debug.hpp"

namespace no {

class script_node_factory {
public:

	const script_node_constructor* find_script_node_constructor(int type) const;
	script_node* create_script_node(int type) const;

	void register_script_node(int type, std::string_view name, std::string_view category, const std::function<script_node*()>& constructor);

	const std::vector<script_node_constructor>& get_core_constructors() const;
	const std::vector<script_node_constructor>& get_user_constructors() const;

private:

	std::vector<script_node_constructor> core_nodes;
	std::vector<script_node_constructor> user_nodes;

};

static script_node_factory node_factory;

const script_node_constructor* script_node_factory::find_script_node_constructor(int type) const {
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

script_node* script_node_factory::create_script_node(int type) const {
	return find_script_node_constructor(type)->construct();
}

void script_node_factory::register_script_node(int type, std::string_view name, std::string_view category, const std::function<script_node*()>& constructor) {
	ASSERT(!find_script_node_constructor(type));
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

script_node_constructor::script_node_constructor(int type, std::string_view name, std::string_view category, const std::function<script_node*()>& constructor)
	: type{ type }, name{ name }, category{ category }, constructor{ constructor } {

}

script_node* script_node_constructor::construct() const {
	return constructor ? constructor() : nullptr;
}

std::optional<int> script_node_constructor::get_type() const {
	return type;
}

std::string_view script_node_constructor::get_name() const {
	return name;
}

std::string_view script_node_constructor::get_category() const {
	return category;
}

bool script_node_constructor::is_valid() const {
	return type.has_value() && constructor;
}

script_node* create_script_node(int type) {
	return node_factory.create_script_node(type);
}

void register_script_node(int id, std::string_view name, std::string_view category, const std::function<script_node*()>& constructor) {
	node_factory.register_script_node(id, name, category, constructor);
}

const std::vector<script_node_constructor>& get_core_script_node_constructors() {
	return node_factory.get_core_constructors();
}

const std::vector<script_node_constructor>& get_user_script_node_constructors() {
	return node_factory.get_user_constructors();
}

std::vector<script_node_constructor> get_all_script_node_constructors() {
	auto constructors = merge_vectors(node_factory.get_core_constructors(), node_factory.get_user_constructors());
	std::erase_if(constructors, [](const auto& constructor) {
		return !constructor.is_valid();
	});
	return constructors;
}

std::optional<int> script_node::process() {
	return std::nullopt;
}

void script_node::write(io_stream& stream) const {
	stream.write<int32_t>(id);
	stream.write_optional<int32_t>(scope_id);
	stream.write(transform);
	stream.write(static_cast<int32_t>(outputs.size()));
	for (const auto& output : outputs) {
		stream.write<int32_t>(output.to_node());
		stream.write<int32_t>(output.slot());
	}
}

void script_node::read(io_stream& stream) {
	id = stream.read<int32_t>();
	scope_id = stream.read_optional<int32_t>();
	transform = stream.read<transform2>();
	const auto out_count = stream.read<int32_t>();
	for (int j{ 0 }; j < out_count; j++) {
		const auto node_id = stream.read<int32_t>();
		const auto out_id = stream.read<int32_t>();
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
	for (int i{ 0 }; i < used_output_slots_count(); i++) {
		if (outputs[i].to_node() == node_id) {
			outputs.erase(outputs.begin() + i);
			i--;
		}
	}
}

void script_node::delete_output_slot(int slot) {
	for (int i{ 0 }; i < used_output_slots_count(); i++) {
		if (outputs[i].slot() == slot) {
			outputs.erase(outputs.begin() + i);
			i--;
		}
	}
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
	for (auto& i : outputs) {
		if (i.slot() == slot.value()) {
			i.set_to_node(to_node_id);
			return;
		}
	}
	outputs.emplace_back(to_node_id, slot.value());
}

bool script_node::has_interactive_output_nodes() const {
	return std::any_of(outputs.begin(), outputs.end(), [this](const auto& output) {
		return tree->get_node(output.to_node())->is_interactive();
	});
}

const std::vector<script_node_output>& script_node::get_outputs() const {
	return outputs;
}

int script_node::used_output_slots_count() const {
	return static_cast<int>(outputs.size());
}

}
