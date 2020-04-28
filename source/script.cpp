#include "script.hpp"
#include "assets.hpp"

namespace no {

struct script_node_register {
	std::unordered_map<int, std::function<script_node*()>> constructors;
};

static script_node_register node_register;

static script_node* create_script_node(int type) {
	auto it = node_register.constructors.find(type);
	return it != node_register.constructors.end() ? it->second() : nullptr;
}

void register_script_node(int type, const std::function<script_node*()>& constructor) {
	ASSERT(node_register.constructors.find(type) == node_register.constructors.end());
	node_register.constructors.emplace(type, constructor);
}

void initialize_scripts() {
	register_script_node<message_node>();
	register_script_node<choice_node>();
	register_script_node<variable_condition_node>();
	register_script_node<modify_variable_node>();
	register_script_node<create_variable_node>();
	register_script_node<variable_exists_node>();
	register_script_node<delete_variable_node>();
	register_script_node<random_node>();
	register_script_node<random_condition_node>();
	register_script_node<execute_node>();
	register_script_node<code_function_node>();
}

void script_node::write(io_stream& stream) const {
	stream.write<int32_t>(id);
	stream.write(transform);
	stream.write<int32_t>(static_cast<int32_t>(out.size()));
	for (const auto& j : out) {
		stream.write<int32_t>(j.node_id);
		stream.write<int32_t>(j.out_id);
	}
}

void script_node::read(io_stream& stream) {
	id = stream.read<int32_t>();
	transform = stream.read<transform2>();
	const int out_count{ stream.read<int32_t>() };
	for (int j{ 0 }; j < out_count; j++) {
		const auto node_id = stream.read<int32_t>();
		const auto out_id = stream.read<int32_t>();
		out.emplace_back(node_id, out_id);
	}
}

void script_node::remove_output_node(int node_id) {
	for (int i{ 0 }; i < static_cast<int>(out.size()); i++) {
		if (out[i].node_id == node_id) {
			out.erase(out.begin() + i);
			i--;
		}
	}
}

void script_node::remove_output_type(int out_id) {
	for (int i{ 0 }; i < static_cast<int>(out.size()); i++) {
		if (out[i].out_id == out_id) {
			out.erase(out.begin() + i);
			i--;
		}
	}
}

int script_node::get_output(int out_id) {
	for (const auto& i : out) {
		if (i.out_id == out_id) {
			return i.node_id;
		}
	}
	return -1;
}

int script_node::get_first_output() {
	return out.empty() ? -1 : out[0].node_id;
}

void script_node::set_output_node(int out_id, int node_id) {
	if (node_id == -1) {
		return;
	}
	if (out_id == -1) {
		out_id = 0;
		while (get_output(out_id) != -1) {
			out_id++;
		}
	}
	for (auto& i : out) {
		if (i.out_id == out_id) {
			i.node_id = node_id;
			return;
		}
	}
	out.emplace_back(node_id, out_id);
}

void script_tree::write(io_stream& stream) const {
	stream.write<int32_t>(id);
	stream.write<int32_t>(static_cast<int32_t>(nodes.size()));
	for (const auto& node : nodes) {
		stream.write<int32_t>(node.second->type());
		node.second->write(stream);
	}
	stream.write<int32_t>(id_counter);
	stream.write<int32_t>(start_node_id);
}

void script_tree::read(io_stream& stream) {
	id = stream.read<int32_t>();
	const auto node_count = stream.read<int32_t>();
	for (int32_t i{ 0 }; i < node_count; i++) {
		const auto type = stream.read<int32_t>();
		auto node = create_script_node(type);
		node->tree = this;
		node->read(stream);
		nodes[node->id] = node;
	}
	id_counter = stream.read<int32_t>();
	start_node_id = stream.read<int32_t>();
}

void script_tree::save() const {
	io_stream stream;
	write(stream);
	file::write(asset_path(STRING("scripts/" << id << ".ed")), stream);
}

void script_tree::load(int id) {
	io_stream stream;
	file::read(asset_path(STRING("scripts/" << id << ".ed")), stream);
	if (stream.size_left_to_read() > 0) {
		read(stream);
	}
}

std::optional<int> script_tree::current_node() const {
	return current_node_id;
}

void script_tree::select_choice(int node_id) {
	if (nodes.find(node_id) == nodes.end()) {
		WARNING("Node not found: " << node_id << ". Script: " << id);
		return;
	}
	current_node_id = nodes[node_id]->get_first_output();
	while (process_choice_selection());
}

void script_tree::process_entry_point() {
	current_node_id = start_node_id;
	while (process_choice_selection());
}

bool script_tree::process_choice_selection() {
	if (!current_node_id.has_value()) {
		return false;
	}
	int type = nodes[current_node_id.value()]->type();
	if (type == message_node::full_type) {
		prepare_message();
		return false;
	}
	if (type == choice_node::full_type) {
		WARNING("A choice cannot be the entry point of a script.");
		return false;
	}
	current_node_id = process_non_interactive_node(current_node_id.value(), type);
	return current_node_id != -1;
}

void script_tree::prepare_message() {
	std::vector<node_choice_info> choice_infos;
	std::vector<int> choices{ process_current_and_get_choices() };
	for (const int choice : choices) {
		choice_infos.push_back({ static_cast<choice_node*>(nodes[choice])->text, choice });
	}
	if (choice_infos.empty()) {
		choice_infos.push_back({ "Oops, I encountered a bug. Gotta go!", -1 });
	}
	events.choice.emit(choice_infos);
}

std::vector<int> script_tree::process_current_and_get_choices() {
	ASSERT(current_node_id.has_value());
	std::vector<int> choices;
	for (const auto& output : nodes[current_node_id.value()]->out) {
		const int out_type{ nodes[output.node_id]->type() };
		if (out_type == choice_node::full_type) {
			choices.push_back(output.node_id);
		} else {
			if (const auto traversed = process_nodes_get_choice(output.node_id, out_type)) {
				choices.push_back(traversed.value());
			}
		}
	}
	return choices;
}

std::optional<int> script_tree::process_nodes_get_choice(std::optional<int> id, int type) {
	while (id.has_value() && type != message_node::full_type) {
		if (type == choice_node::full_type) {
			return id;
		}
		id = process_non_interactive_node(id.value(), type).value_or(-1);
		if (id.has_value()) {
			type = nodes[id.value()]->type();
			if (type == choice_node::full_type) {
				return id;
			}
		}
	}
	return std::nullopt;
}

std::optional<int> script_tree::process_non_interactive_node(int id, int type) {
	auto node = nodes[id];
	if (const int out{ node->process() }; out >= 0) {
		return node->get_output(out);
	} else {
		return std::nullopt;
	}
}

void message_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write(text);
}

void message_node::read(io_stream& stream) {
	script_node::read(stream);
	text = stream.read<std::string>();
}

void choice_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write(text);
}

void choice_node::read(io_stream& stream) {
	script_node::read(stream);
	text = stream.read<std::string>();
}

int variable_condition_node::process() {
	auto context = tree->context;
	const auto* variable = context->find(is_global ? std::optional<int>{} : scope_id, variable_name);
	if (!variable) {
		WARNING("Attempted to check " << variable_name << " (global: " << is_global << ") but it does not exist");
		return false;
	}
	if (comparison_value == "") {
		return variable->name == "";
	}
	std::string value{ comparison_value };
	if (other_type == node_other_variable_type::local) {
		if (const auto* local_variable = context->find(scope_id, comparison_value)) {
			value = local_variable->value;
		} else {
			WARNING("Cannot compare against " << variable_name << " because local variable " << comparison_value << " does not exist.");
			return false;
		}
	} else if (other_type == node_other_variable_type::global) {
		if (const auto* global_variable = context->find(std::nullopt, comparison_value)) {
			value = global_variable->value;
		} else {
			WARNING("Cannot compare against " << variable_name << " because global variable " << comparison_value << " does not exist.");
			return false;
		}
	}
	return variable->compare(value, comparison_operator) ? 1 : 0;
}

void variable_condition_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write<int32_t>(static_cast<int32_t>(other_type));
	stream.write(variable_name);
	stream.write(comparison_value);
	stream.write<int32_t>(static_cast<int32_t>(comparison_operator));
}

void variable_condition_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = (stream.read<uint8_t>() != 0);
	other_type = static_cast<node_other_variable_type>(stream.read<int32_t>());
	variable_name = stream.read<std::string>();
	comparison_value = stream.read<std::string>();
	comparison_operator = static_cast<variable_comparison>(stream.read<int32_t>());
}

int modify_variable_node::process() {
	if (modify_value == "") {
		return 0;
	}
	auto context = tree->context;
	auto variable = context->find(is_global ? std::optional<int>{} : scope_id, variable_name);
	if (!variable) {
		WARNING("Attempted to modify " << variable_name << " (global: " << is_global << ") but it does not exist");
		return 0;
	}
	std::string value{ modify_value };
	if (other_type == node_other_variable_type::local) {
		if (const auto* local_variable = context->find(scope_id, modify_value)) {
			value = local_variable->value;
		} else {
			WARNING("Cannot modify " << variable_name << " because the local variable " << modify_value << " does not exist.");
			return 0;
		}
	} else if (other_type == node_other_variable_type::global) {
		if (const auto* global_variable = context->find(std::nullopt, modify_value)) {
			value = global_variable->value;
		} else {
			WARNING("Cannot modify " << variable_name << " because the global variable " << modify_value << " does not exist.");
			return 0;
		}
	}
	variable->modify(value, modify_operator);
	return 0;
}

void modify_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write(static_cast<int32_t>(other_type));
	stream.write(variable_name);
	stream.write(modify_value);
	stream.write(static_cast<int32_t>(modify_operator));
}

void modify_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = (stream.read<uint8_t>() != 0);
	other_type = static_cast<node_other_variable_type>(stream.read<int32_t>());
	variable_name = stream.read<std::string>();
	modify_value = stream.read<std::string>();
	modify_operator = static_cast<variable_modification>(stream.read<int32_t>());
}

int create_variable_node::process() {
	auto context = tree->context;
	if (auto existing_variable = context->find(is_global ? std::optional<int>{} : scope_id, new_variable.name)) {
		if (overwrite) {
			*existing_variable = new_variable;
		}
	} else {
		context->add(is_global ? std::optional<int>{} : scope_id, new_variable);
	}
	return 0;
}

void create_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write<uint8_t>(overwrite);
	stream.write((int32_t)new_variable.type);
	stream.write(new_variable.name);
	stream.write(new_variable.value);
	stream.write<uint8_t>(new_variable.persistent);
}

void create_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = (stream.read<uint8_t>() != 0);
	overwrite = (stream.read<uint8_t>() != 0);
	new_variable.type = static_cast<variable_type>(stream.read<int32_t>());
	new_variable.name = stream.read<std::string>();
	new_variable.value = stream.read<std::string>();
	new_variable.persistent = (stream.read<uint8_t>() != 0);
}

int variable_exists_node::process() {
	return tree->context->find(is_global ? std::optional<int>{} : scope_id, variable_name) ? 1 : 0;
}

void variable_exists_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = (stream.read<uint8_t>() != 0);
	variable_name = stream.read<std::string>();
}

void variable_exists_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write(variable_name);
}

int delete_variable_node::process() {
	tree->context->remove(is_global ? std::optional<int>{} : scope_id, variable_name);
	return 0;
}

void delete_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write(variable_name);
}

void delete_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = (stream.read<uint8_t>() != 0);
	variable_name = stream.read<std::string>();
}

int random_node::process() {
	if (out.empty()) {
		WARNING("No nodes attached.");
		return -1;
	}
	// todo: randomness
	return 0;
}

void random_node::write(io_stream& stream) const {
	script_node::write(stream);
}

void random_node::read(io_stream& stream) {
	script_node::read(stream);
}

int random_condition_node::process() {
	// todo: randomness
	return 0;
}

void random_condition_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<int32_t>(percent);
}

void random_condition_node::read(io_stream& stream) {
	script_node::read(stream);
	percent = stream.read<int32_t>();
}

int execute_node::process() {
	return 0;
}

void execute_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write(script);
}

void execute_node::read(io_stream& stream) {
	script_node::read(stream);
	script = stream.read<std::string>();
}

}
