#include "graphics/ui.hpp"
#include "scripts/nodes/choice_node.hpp"
#include "scripts/nodes/compare_variable_node.hpp"
#include "scripts/nodes/create_variable_node.hpp"
#include "scripts/nodes/delete_variable_node.hpp"
#include "scripts/nodes/execute_script_node.hpp"
#include "scripts/nodes/message_node.hpp"
#include "scripts/nodes/modify_variable_node.hpp"
#include "scripts/nodes/random_condition_node.hpp"
#include "scripts/nodes/trigger_event_node.hpp"
#include "scripts/nodes/variable_exists_node.hpp"
#include "math.hpp"
#include "objects/objects.hpp"
#include "scripts/nodes/spawn_object_node.hpp"
#include "scripts/script_tree.hpp"

namespace nfwk {

bool choice_node_update_editor(choice_node& node) {
	return ui::input(u8"##choice", node.text, { 360.0f, 50.0f });
}

bool compare_variable_node_update_editor(compare_variable_node& node) {
	bool dirty = ui::checkbox(u8"Global", node.is_global);
	dirty |= ui::input(u8"Name", node.variable_name);
	if (auto new_comparison = ui::combo(u8"Comparison", { u8"==", u8"!=", u8">", u8"<", u8">=", u8"<=" }, static_cast<int>(node.comparison_operator))) {
		node.comparison_operator = static_cast<variable_comparison>(new_comparison.value());
		dirty = true;
	}
	if (auto new_type = ui::combo(u8"##other-type", { u8"Value", u8"Local", u8"Global" }, static_cast<int>(node.other_type))) {
		node.other_type = static_cast<other_variable_type>(new_type.value());
		dirty = true;
	}
	dirty |= ui::input(u8"Value", node.comparison_value);
	return dirty;
}

bool create_variable_node_update_editor(create_variable_node& node) {
	bool dirty = ui::checkbox(u8"Global", node.is_global);
	dirty |= ui::checkbox(u8"Persistent", node.new_variable.persistent);
	dirty |= ui::checkbox(u8"Overwrite", node.overwrite);
	if (auto index = ui::combo(u8"Type", { u8"String", u8"Integer", u8"Boolean", u8"Float" }, static_cast<int>(node.new_variable.type))) {
		node.new_variable.type = static_cast<variable_type>(index.value());
		switch (node.new_variable.type) {
		case variable_type::string:
			node.new_variable.value = u8"";
			break;
		case variable_type::integer:
		case variable_type::boolean:
			node.new_variable.value = 0;
			break;
		case variable_type::floating:
			node.new_variable.value = 0.0f;
			break;
		}
		dirty = true;
	}
	dirty |= ui::input(u8"Name", node.new_variable.name);
	if (node.new_variable.type == variable_type::string) {
		dirty |= ui::input(u8"Value##string", std::get<std::string>(node.new_variable.value));
	} else if (node.new_variable.type == variable_type::integer) {
		dirty |= ui::input(u8"Value##integer", std::get<int>(node.new_variable.value));
	} else if (node.new_variable.type == variable_type::floating) {
		dirty |= ui::input(u8"Value##float", std::get<float>(node.new_variable.value));
	} else if (node.new_variable.type == variable_type::boolean) {
		bool as_bool = (std::get<int>(node.new_variable.value) != 0);
		dirty |= ui::checkbox(u8"Value##bool", as_bool);
		node.new_variable.value = as_bool ? 1 : 0;
	}
	return dirty;
}

bool delete_variable_node_update_editor(delete_variable_node& node) {
	bool dirty = ui::checkbox(u8"Global", node.is_global);
	dirty |= ui::input(u8"Name", node.variable_name);
	return dirty;
}

bool execute_script_node_update_editor(execute_script_node& node) {
	bool dirty = ui::input(u8"Script", node.script_id);
	return dirty;
}

bool message_node_update_editor(message_node& node) {
	return ui::input(u8"##message", node.text, { 360.0f, 160.0f });
}

bool modify_variable_node_update_editor(modify_variable_node& node) {
	bool dirty = ui::checkbox(u8"Global", node.is_global);
	dirty |= ui::input(u8"Name", node.variable_name);
	if (auto new_operator = ui::combo(u8"Operator", { u8"Set", u8"Negate", u8"Add", u8"Multiply", u8"Divide" }, static_cast<int>(node.modify_operator))) {
		node.modify_operator = static_cast<variable_operator>(new_operator.value());
		dirty = true;
	}
	if (auto new_type = ui::combo(u8"##other-type", { u8"Value", u8"Local", u8"Global" }, static_cast<int>(node.other_type))) {
		node.other_type = static_cast<other_variable_type>(new_type.value());
		dirty = true;
	}
	dirty |= ui::input(u8"##set-value", node.modify_value);
	return dirty;
}

bool random_condition_node_update_editor(random_condition_node& node) {
	bool dirty = ui::input(u8"% Chance of success", node.percent);
	node.percent = clamp(node.percent, 0, 100);
	return dirty;
}

bool random_output_node_update_editor() {
	return false;
}

bool spawn_object_node_update_editor(spawn_object_node& node) {
	bool dirty{ false };
	//auto selected_result = class_search.update([&](std::string search_term, int limit) {
	//	return node.get_tree().context.objects->search_class(normalized_identifier(search_term), limit);
	//});
	//if (selected_result.has_value()) {
		//node.selected_class_index = selected_result.value()->get_index();
	//}
	dirty |= ui::input(u8"##position", node.position);
	return dirty;
}

bool trigger_event_node_update_editor(trigger_event_node& node) {
	bool dirty = ui::input(u8"##event-id", node.event_id);
	if (!node.get_tree().context.events->exists(node.event_id)) {
		ui::colored_text({ 1.0f, 0.4f, 0.4f }, u8"Event does not exist.");
	}
	return dirty;
}

bool variable_exists_node_update_editor(variable_exists_node& node) {
	bool dirty = ui::checkbox(u8"Global", node.is_global);
	dirty |= ui::input(u8"Name", node.variable_name);
	return dirty;
}

}
