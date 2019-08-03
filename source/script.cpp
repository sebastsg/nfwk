#include "script.hpp"
#include "assets.hpp"

#define NO_SCRIPT_NODE_MESSAGE 0
#define NO_SCRIPT_NODE_CHOICE  1

namespace no {

struct script_node_register {
	std::unordered_map<int, std::function<script_node*()>> constructors;
};

static script_node_register node_register;

static script_node* create_script_node(int type) {
	auto it = node_register.constructors.find(type);
	return it != node_register.constructors.end() ? it->second() : nullptr;
}

void script_node::write(io_stream& stream) {
	stream.write<int32_t>(id);
	stream.write(transform);
	stream.write((int32_t)out.size());
	for (auto& j : out) {
		stream.write<int32_t>(j.out_id);
		stream.write<int32_t>(j.node_id);
	}
}

void script_node::read(io_stream& stream) {
	id = stream.read<int32_t>();
	transform = stream.read<no::transform3>();
	int out_count = stream.read<int32_t>();
	for (int j = 0; j < out_count; j++) {
		node_output output;
		output.out_id = stream.read<int32_t>();
		output.node_id = stream.read<int32_t>();
		out.push_back(output);
	}
}

void script_node::remove_output_node(int node_id) {
	for (int i = 0; i < (int)out.size(); i++) {
		if (out[i].node_id == node_id) {
			out.erase(out.begin() + i);
			i--;
		}
	}
}

void script_node::remove_output_type(int out_id) {
	for (int i = 0; i < (int)out.size(); i++) {
		if (out[i].out_id == out_id) {
			out.erase(out.begin() + i);
			i--;
		}
	}
}

int script_node::get_output(int out_id) {
	for (auto& i : out) {
		if (i.out_id == out_id) {
			return i.node_id;
		}
	}
	return -1;
}

int script_node::get_first_output() {
	if (out.empty()) {
		return -1;
	}
	return out[0].node_id;
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
	node_output output;
	output.node_id = node_id;
	output.out_id = out_id;
	out.push_back(output);
}

void script_tree::write(io_stream& stream) const {
	stream.write<int32_t>(id);
	stream.write((int32_t)nodes.size());
	for (auto& i : nodes) {
		stream.write((int32_t)i.second->type());
		i.second->write(stream);
	}
	stream.write<int32_t>(id_counter);
	stream.write<int32_t>(start_node_id);
}

void script_tree::read(no::io_stream & stream) {
	id = stream.read<int32_t>();
	int node_count = stream.read<int32_t>();
	for (int i = 0; i < node_count; i++) {
		int type = stream.read<int32_t>();
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

int script_tree::current_node() const {
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
	if (current_node_id == -1) {
		return false;
	}
	int type = nodes[current_node_id]->type();
	if (type == NO_SCRIPT_NODE_MESSAGE) {
		prepare_message();
		return false;
	}
	if (type == NO_SCRIPT_NODE_CHOICE) {
		INFO("A choice cannot be the entry point of a script.");
		return false;
	}
	current_node_id = process_non_ui_node(current_node_id, type);
	return current_node_id != -1;
}

void script_tree::prepare_message() {
	choice_event event;
	std::vector<int> choices = process_current_and_get_choices();
	for (int choice : choices) {
		event.choices.push_back({ ((choice_node*)nodes[choice])->text, choice });
	}
	if (event.choices.empty()) {
		event.choices.push_back({ "Oops, I encountered a bug. Gotta go!", -1 });
	}
	events.choice.emit(event);
}

std::vector<int> script_tree::process_current_and_get_choices() {
	std::vector<int> choices;
	for (auto& output : nodes[current_node_id]->out) {
		int out_type = nodes[output.node_id]->type();
		if (out_type == NO_SCRIPT_NODE_CHOICE) {
			choices.push_back(output.node_id);
		} else {
			int traversed = process_nodes_get_choice(output.node_id, out_type);
			if (traversed != -1) {
				choices.push_back(traversed);
			}
		}
	}
	return choices;
}

int script_tree::process_nodes_get_choice(int id, int type) {
	while (true) {
		if (id == -1 || type == NO_SCRIPT_NODE_MESSAGE) {
			return -1;
		}
		if (type == NO_SCRIPT_NODE_CHOICE) {
			return id;
		}
		id = process_non_ui_node(id, type);
		if (id == -1) {
			return -1;
		}
		type = nodes[id]->type();
		if (type == NO_SCRIPT_NODE_CHOICE) {
			return id;
		}
	}
	return -1;
}

int script_tree::process_non_ui_node(int id, int type) {
	script_node* node = nodes[id];
	int out = node->process();
	if (out == -1) {
		return -1;
	}
	return node->get_output(out);
}

void message_node::write(no::io_stream & stream) {
	script_node::write(stream);
	stream.write(text);
}

void message_node::read(no::io_stream & stream) {
	script_node::read(stream);
	text = stream.read<std::string>();
}

void choice_node::write(no::io_stream & stream) {
	script_node::write(stream);
	stream.write(text);
}

void choice_node::read(no::io_stream & stream) {
	script_node::read(stream);
	text = stream.read<std::string>();
}


int var_condition_node::process() {
	variable_map* variables = tree->variables;
	variable* var = nullptr;
	if (is_global) {
		var = variables->global(var_name);
	} else {
		var = variables->local(scope_id, var_name);
	}
	if (!var) {
		WARNING("Attempted to check " << var_name << " (global: " << is_global << ") but it does not exist");
		return false;
	}
	if (comp_value == "") {
		return var->name == "";
	}
	std::string value = comp_value;
	if (other_type == node_other_var_type::local) {
		variable* comp_var = variables->local(scope_id, comp_value);
		if (comp_var) {
			value = comp_var->value;
		} else {
			WARNING("Cannot compare against " << var_name << " because local variable " << comp_value << " does not exist.");
			return false;
		}
	} else if (other_type == node_other_var_type::global) {
		auto comp_var = variables->global(comp_value);
		if (comp_var) {
			value = comp_var->value;
		} else {
			WARNING("Cannot compare against " << var_name << " because global variable " << comp_value << " does not exist.");
			return false;
		}
	}
	return var->compare(value, comp_operator) ? 1 : 0;
}

void var_condition_node::write(io_stream& stream) {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write((int32_t)other_type);
	stream.write(var_name);
	stream.write(comp_value);
	stream.write((int32_t)comp_operator);
}

void var_condition_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = (stream.read<uint8_t>() != 0);
	other_type = (node_other_var_type)stream.read<int32_t>();
	var_name = stream.read<std::string>();
	comp_value = stream.read<std::string>();
	comp_operator = (variable_comparison)stream.read<int32_t>();
}

int modify_var_node::process() {
	if (mod_value == "") {
		return 0;
	}
	variable_map* variables = tree->variables;
	variable* var = nullptr;
	if (is_global) {
		var = variables->global(var_name);
	} else {
		var = variables->local(scope_id, var_name);
	}
	if (!var) {
		WARNING("Attempted to modify " << var_name << " (global: " << is_global << ") but it does not exist");
		return 0;
	}
	std::string value = mod_value;
	if (other_type == node_other_var_type::local) {
		auto mod_var = variables->local(scope_id, mod_value);
		if (mod_var) {
			value = mod_var->value;
		} else {
			WARNING("Cannot modify " << var_name << " because the local variable " << mod_value << " does not exist.");
			return 0;
		}
	} else if (other_type == node_other_var_type::global) {
		auto mod_var = variables->global(mod_value);
		if (mod_var) {
			value = mod_var->value;
		} else {
			WARNING("Cannot modify " << var_name << " because the global variable " << mod_value << " does not exist.");
			return 0;
		}
	}
	var->modify(value, mod_operator);
	return 0;
}

void modify_var_node::write(io_stream& stream) {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write((int32_t)other_type);
	stream.write(var_name);
	stream.write(mod_value);
	stream.write((int32_t)mod_operator);
}

void modify_var_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = (stream.read<uint8_t>() != 0);
	other_type = (node_other_var_type)stream.read<int32_t>();
	var_name = stream.read<std::string>();
	mod_value = stream.read<std::string>();
	mod_operator = (variable_modification)stream.read<int32_t>();
}

int create_var_node::process() {
	variable_map* variables = tree->variables;
	if (is_global) {
		variable* old_var = variables->global(var.name);
		if (old_var) {
			if (overwrite) {
				*old_var = var;
			}
			return 0;
		}
		variables->create_global(var);
	} else {
		variable* old_var = variables->local(scope_id, var.name);
		if (old_var) {
			if (overwrite) {
				*old_var = var;
			}
			return 0;
		}
		variables->create_local(scope_id, var);
	}
	return 0;
}

void create_var_node::write(io_stream& stream) {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write<uint8_t>(overwrite);
	stream.write((int32_t)var.type);
	stream.write(var.name);
	stream.write(var.value);
	stream.write<uint8_t>(var.is_persistent);
}

void create_var_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = (stream.read<uint8_t>() != 0);
	overwrite = (stream.read<uint8_t>() != 0);
	var.type = (variable_type)stream.read<int32_t>();
	var.name = stream.read<std::string>();
	var.value = stream.read<std::string>();
	var.is_persistent = (stream.read<uint8_t>() != 0);
}

int var_exists_node::process() {
	if (is_global) {
		return tree->variables->global(var_name) != nullptr ? 1 : 0;
	}
	return tree->variables->local(scope_id, var_name) != nullptr ? 1 : 0;
}

void var_exists_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = (stream.read<uint8_t>() != 0);
	var_name = stream.read<std::string>();
}

void var_exists_node::write(io_stream& stream) {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write(var_name);
}

int delete_var_node::process() {
	if (is_global) {
		tree->variables->delete_global(var_name);
	} else {
		tree->variables->delete_local(scope_id, var_name);
	}
	return 0;
}

void delete_var_node::write(io_stream& stream) {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write(var_name);
}

void delete_var_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = (stream.read<uint8_t>() != 0);
	var_name = stream.read<std::string>();
}

int random_node::process() {
	if (out.size() == 0) {
		WARNING("No nodes attached.");
		return -1;
	}
	// todo: randomness
	return 0;
}

void random_node::write(io_stream& stream) {
	script_node::write(stream);
}

void random_node::read(io_stream& stream) {
	script_node::read(stream);
}

int random_condition_node::process() {
	// todo: randomness
	return 0;
}

void random_condition_node::write(no::io_stream & stream) {
	script_node::write(stream);
	stream.write<int32_t>(percent);
}

void random_condition_node::read(no::io_stream & stream) {
	script_node::read(stream);
	percent = stream.read<int32_t>();
}

}
