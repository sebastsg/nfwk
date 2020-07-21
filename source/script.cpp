#include "script.hpp"
#include "assets.hpp"
#include "script_editor.hpp"
#include "ui.hpp"

namespace no::internal {

void initialize_scripts() {
	register_script_node<message_node>();
	register_script_node<choice_node>();
	register_script_node<compare_variable_node>();
	register_script_node<modify_variable_node>();
	register_script_node<create_variable_node>();
	register_script_node<variable_exists_node>();
	register_script_node<delete_variable_node>();
	register_script_node<random_output_node>();
	register_script_node<random_condition_node>();
	register_script_node<execute_node>();
	register_script_node<code_function_node>();
	register_editor<script_editor>();
}

}

namespace no {

static std::vector<script_node_constructor> registered_nodes;

const script_node_constructor* find_script_node_constructor(int type) {
	for (const auto& node : registered_nodes) {
		if (node.type == type) {
			return &node;
		}
	}
	return nullptr;
}

static script_node* create_script_node(int type) {
	const auto* node = find_script_node_constructor(type);
	return node ? node->constructor() : nullptr;
}

void register_script_node(int type, std::string_view name, std::string_view category, const std::function<script_node*()>& constructor) {
	ASSERT(!find_script_node_constructor(type));
	registered_nodes.emplace_back(script_node_constructor{ constructor, name, category, type });
}

const std::vector<script_node_constructor>& get_registered_script_nodes() {
	return registered_nodes;
}

node_output::node_output(int to_node_id, int slot_index)
	: to_node_id{ to_node_id }, slot_index{ slot_index } {
	ASSERT(to_node_id >= 0);
}

int node_output::slot() const {
	return slot_index;
}

int node_output::to_node() const {
	return to_node_id;
}

void node_output::set_to_node(int to_node) {
	to_node_id = to_node;
}

void script_node::write(io_stream& stream) const {
	stream.write<int32_t>(id);
	stream.write<bool>(scope_id.has_value());
	if (scope_id.has_value()) {
		stream.write<int32_t>(scope_id.value());
	}
	stream.write(transform);
	stream.write(static_cast<int32_t>(outputs.size()));
	for (const auto& output : outputs) {
		stream.write<int32_t>(output.to_node());
		stream.write<int32_t>(output.slot());
	}
}

void script_node::read(io_stream& stream) {
	id = stream.read<int32_t>();
	scope_id = std::nullopt;
	if (stream.read<bool>()) {
		scope_id = stream.read<int32_t>();
	}
	transform = stream.read<transform2>();
	const auto out_count = stream.read<int32_t>();
	for (int j{ 0 }; j < out_count; j++) {
		const auto node_id = stream.read<int32_t>();
		const auto out_id = stream.read<int32_t>();
		outputs.emplace_back(node_id, out_id);
	}
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
	for (const auto& i : outputs) {
		if (i.slot() == slot) {
			return i.to_node();
		}
	}
	return std::nullopt;
}

std::optional<node_output> script_node::get_first_output() const {
	return outputs.empty() ? std::optional<node_output>{} : outputs[0];
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

const std::vector<node_output>& script_node::get_outputs() const {
	return outputs;
}

int script_node::used_output_slots_count() const {
	return static_cast<int>(outputs.size());
}

void script_tree::write(io_stream& stream) const {
	stream.write(id);
	stream.write(name);
	stream.write<int32_t>(static_cast<int32_t>(nodes.size()));
	for (const auto& node : nodes) {
		stream.write<int32_t>(node.second->type());
		node.second->write(stream);
	}
	stream.write<int32_t>(id_counter);
	stream.write<int32_t>(start_node_id);
}

void script_tree::read(io_stream& stream) {
	id = stream.read<std::string>();
	name = stream.read<std::string>();
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
	file::write(asset_path("scripts/" + id + ".ns"), stream);
}

void script_tree::load(const std::string& id) {
	io_stream stream;
	file::read(asset_path("scripts/" + id + ".ns"), stream);
	if (stream.size_left_to_read() > 0) {
		read(stream);
	}
}

std::optional<int> script_tree::current_node() const {
	return current_node_id;
}

void script_tree::select_choice(int node_id) {
	if (const auto node = nodes.find(node_id); node != nodes.end()) {
		const auto output = node->second->get_first_output();
		current_node_id = output ? output->to_node() : std::optional<int>{};
		while (process_choice_selection());
	} else {
		WARNING_X("scripts", "Node not found: " << node_id << ". Script: " << id);
	}
}

void script_tree::process_entry_point() {
	current_node_id = start_node_id;
	while (process_choice_selection());
}

bool script_tree::process_choice_selection() {
	if (!current_node_id.has_value()) {
		return false;
	}
	const auto type = nodes[current_node_id.value()]->type();
	if (type == message_node::full_type) {
		prepare_message();
		return false;
	}
	if (type == choice_node::full_type) {
		WARNING_X("scripts", "A choice cannot be the entry point of a script.");
		return false;
	}
	current_node_id = process_non_interactive_node(current_node_id.value(), type);
	return current_node_id.has_value();
}

void script_tree::prepare_message() {
	std::vector<node_choice_info> choice_infos;
	for (const int choice : process_current_and_get_choices()) {
		if (const auto* node = dynamic_cast<const choice_node*>(nodes[choice])) {
			choice_infos.emplace_back(node->text, choice);
		} else {
			BUG(choice << " is not a choice node!");
		}
	}
	if (choice_infos.empty()) {
		choice_infos.emplace_back("Oops, I encountered a bug. Gotta go!", std::nullopt);
	}
	events.choice.emit(choice_infos);
}

std::vector<int> script_tree::process_current_and_get_choices() {
	ASSERT(current_node_id.has_value());
	std::vector<int> choices;
	for (const auto& output : nodes[current_node_id.value()]->outputs) {
		const int out_type{ nodes[output.to_node()]->type() };
		if (out_type == choice_node::full_type) {
			choices.push_back(output.to_node());
		} else {
			if (const auto traversed = process_nodes_get_choice(output.to_node(), out_type)) {
				choices.push_back(traversed.value());
			}
		}
	}
	return choices;
}

std::optional<int> script_tree::process_nodes_get_choice(std::optional<int> node_id, int type) {
	while (node_id.has_value() && type != message_node::full_type) {
		if (type == choice_node::full_type) {
			return node_id;
		}
		node_id = process_non_interactive_node(node_id.value(), type);
		if (node_id.has_value()) {
			type = nodes[node_id.value()]->type();
			if (type == choice_node::full_type) {
				return node_id;
			}
		}
	}
	return std::nullopt;
}

std::optional<int> script_tree::process_non_interactive_node(int node_id, int type) {
	auto node = nodes[node_id];
	if (const auto slot = node->process()) {
		return node->get_output_node(slot.value());
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

std::optional<int> compare_variable_node::process() {
	auto context = tree->context;
	const auto* variable = context->find(scope_id, variable_name);
	if (!variable) {
		WARNING_X("scripts", "Attempted to check " << variable_name << " (global: " << is_global << ") but it does not exist");
		return std::nullopt;
	}
	if (comparison_value == "") {
		return variable->name == "" ? 1 : 0;
	}
	std::string value{ comparison_value };
	if (other_type == node_other_variable_type::local) {
		if (const auto* local_variable = context->find(scope_id, comparison_value)) {
			value = local_variable->value;
		} else {
			WARNING_X("scripts", "Cannot compare against " << variable_name << " because local variable " << comparison_value << " does not exist.");
			return 0;
		}
	} else if (other_type == node_other_variable_type::global) {
		if (const auto* global_variable = context->find(std::nullopt, comparison_value)) {
			value = global_variable->value;
		} else {
			WARNING_X("scripts", "Cannot compare against " << variable_name << " because global variable " << comparison_value << " does not exist.");
			return 0;
		}
	}
	return variable->compare(value, comparison_operator) ? 1 : 0;
}

void compare_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write<int32_t>(static_cast<int32_t>(other_type));
	stream.write(variable_name);
	stream.write(comparison_value);
	stream.write<int32_t>(static_cast<int32_t>(comparison_operator));
}

void compare_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = (stream.read<uint8_t>() != 0);
	other_type = static_cast<node_other_variable_type>(stream.read<int32_t>());
	variable_name = stream.read<std::string>();
	comparison_value = stream.read<std::string>();
	comparison_operator = static_cast<variable_comparison>(stream.read<int32_t>());
}

std::optional<int> modify_variable_node::process() {
	if (modify_value == "") {
		return 0;
	}
	auto context = tree->context;
	auto variable = context->find(scope_id, variable_name);
	if (!variable) {
		WARNING_X("scripts", "Attempted to modify " << variable_name << " (global: " << is_global << ") but it does not exist");
		return 0;
	}
	std::string value{ modify_value };
	if (other_type == node_other_variable_type::local) {
		if (const auto* local_variable = context->find(scope_id, modify_value)) {
			value = local_variable->value;
		} else {
			WARNING_X("scripts", "Cannot modify " << variable_name << " because the local variable " << modify_value << " does not exist.");
			return 0;
		}
	} else if (other_type == node_other_variable_type::global) {
		if (const auto* global_variable = context->find(std::nullopt, modify_value)) {
			value = global_variable->value;
		} else {
			WARNING_X("scripts", "Cannot modify " << variable_name << " because the global variable " << modify_value << " does not exist.");
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

std::optional<int> create_variable_node::process() {
	auto context = tree->context;
	if (auto existing_variable = context->find(scope_id, new_variable.name)) {
		if (overwrite) {
			*existing_variable = new_variable;
		}
	} else {
		context->add(scope_id, new_variable);
	}
	return 0;
}

void create_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<uint8_t>(is_global);
	stream.write<uint8_t>(overwrite);
	stream.write(static_cast<int32_t>(new_variable.type));
	stream.write(new_variable.name);
	stream.write(new_variable.value);
	stream.write<uint8_t>(new_variable.persistent);
}

void create_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read<bool>();
	overwrite = stream.read<bool>();
	new_variable.type = static_cast<variable_type>(stream.read<int32_t>());
	new_variable.name = stream.read<std::string>();
	new_variable.value = stream.read<std::string>();
	new_variable.persistent = stream.read<bool>();
}

std::optional<int> variable_exists_node::process() {
	return tree->context->find(scope_id, variable_name) ? 1 : 0;
}

void variable_exists_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read<bool>();
	variable_name = stream.read<std::string>();
}

void variable_exists_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<bool>(is_global);
	stream.write(variable_name);
}

std::optional<int> delete_variable_node::process() {
	tree->context->remove(scope_id, variable_name);
	return 0;
}

void delete_variable_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write<bool>(is_global);
	stream.write(variable_name);
}

void delete_variable_node::read(io_stream& stream) {
	script_node::read(stream);
	is_global = stream.read<bool>();
	variable_name = stream.read<std::string>();
}

std::optional<int> random_output_node::process() {
	if (outputs.empty()) {
		WARNING_X("scripts", "No nodes attached.");
		return std::nullopt;
	} else {
		return 0;
	}
}

void random_output_node::write(io_stream& stream) const {
	script_node::write(stream);
}

void random_output_node::read(io_stream& stream) {
	script_node::read(stream);
}

std::optional<int> random_condition_node::process() {
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

std::optional<int> execute_node::process() {
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

bool message_node::update_editor() {
	return ui::input("##message", text, { 350.0f, ImGui::GetTextLineHeight() * 8.0f });
}

bool choice_node::update_editor() {
	return ui::input("##choice", text, { 350.0f, ImGui::GetTextLineHeight() * 3.0f });
}

bool compare_variable_node::update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::input("Name", variable_name);
	if (auto new_comparison = ui::combo("Comparison", { "==", "!=", ">", "<", ">=", "<=" }, static_cast<int>(comparison_operator))) {
		comparison_operator = static_cast<variable_comparison>(new_comparison.value());
		dirty = true;
	}
	if (auto new_type = ui::combo("##other-type", { "Value", "Local", "Global" }, static_cast<int>(other_type))) {
		other_type = static_cast<node_other_variable_type>(new_type.value());
		dirty = true;
	}
	dirty |= ui::input("Value", comparison_value);
	return dirty;
}

bool modify_variable_node::update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::input("Name", variable_name);
	if (auto new_operator = ui::combo("Operator", { "Set", "Negate", "Add", "Multiply", "Divide" }, static_cast<int>(modify_operator))) {
		modify_operator = static_cast<variable_modification>(new_operator.value());
		dirty = true;
	}
	if (auto new_type = ui::combo("##other-type", { "Value", "Local", "Global" }, static_cast<int>(other_type))) {
		other_type = static_cast<node_other_variable_type>(new_type.value());
		dirty = true;
	}
	dirty |= ui::input("##set-value", modify_value);
	return dirty;
}

bool create_variable_node::update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::checkbox("Persistent", new_variable.persistent);
	dirty |= ui::checkbox("Overwrite", overwrite);
	if (auto new_type = ui::combo("Type", { "String", "Integer", "Float", "Boolean" }, static_cast<int>(new_variable.type))) {
		new_variable.type = static_cast<variable_type>(new_type.value());
		dirty = true;
	}
	dirty |= ui::input("Name", new_variable.name);
	if (new_variable.type == variable_type::string) {
		dirty |= ui::input("Value##string", new_variable.value);
	} else {
		if (new_variable.value == "") {
			new_variable.value = "0";
		}
		if (new_variable.type == variable_type::integer) {
			int as_int = std::stoi(new_variable.value);
			dirty |= ui::input("Value##integer", as_int);
			new_variable.value = std::to_string(as_int);
		} else if (new_variable.type == variable_type::floating) {
			float as_float = std::stof(new_variable.value);
			dirty |= ui::input("Value##float", as_float);
			new_variable.value = std::to_string(as_float);
		} else if (new_variable.type == variable_type::boolean) {
			bool as_bool = (std::stoi(new_variable.value) != 0);
			dirty |= ui::checkbox("Value##bool", as_bool);
			new_variable.value = std::to_string(as_bool);
		}
	}
	return dirty;
}

bool variable_exists_node::update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::input("Name", variable_name);
	return dirty;
}

bool delete_variable_node::update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::input("Name", variable_name);
	return dirty;
}

bool random_output_node::update_editor() {
	return false;
}

bool random_condition_node::update_editor() {
	bool dirty = ui::input("% Chance of success", percent);
	percent = clamp(percent, 0, 100);
	return dirty;
}

bool execute_node::update_editor() {
	return false;
}

}
