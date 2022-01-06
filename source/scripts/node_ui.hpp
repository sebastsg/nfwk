#pragma once

#include "scripts/nodes/choice_node.hpp"
#include "scripts/nodes/execute_script_node.hpp"
#include "scripts/nodes/message_node.hpp"
#include "scripts/nodes/random_condition_node.hpp"
#include "scripts/nodes/trigger_event_node.hpp"
#include "scripts/nodes/variable_nodes.hpp"
#include "scripts/nodes/spawn_object_node.hpp"
#include "scripts/nodes/random_output_node.hpp"
#include "objects/objects.hpp"
#include "graphics/ui.hpp"
#include "math.hpp"

namespace nfwk::script {

bool choice_node_update_editor(choice_node& node) {
	return ui::input("##choice", node.text, { 360.0f, 50.0f });
}

bool compare_variable_node_update_editor(compare_variable_node& node) {
	bool dirty = ui::checkbox("Global", node.is_global);
	dirty |= ui::input("Name", node.variable_name);
	if (auto new_comparison = ui::combo("Comparison", { "==", "!=", ">", "<", ">=", "<=" }, static_cast<int>(node.comparison_operator))) {
		node.comparison_operator = static_cast<variable_comparison>(new_comparison.value());
		dirty = true;
	}
	if (auto new_type = ui::combo("##other-type", { "value", "local", "global" }, static_cast<int>(node.other_type))) {
		node.other_type = static_cast<other_variable_type>(new_type.value());
		dirty = true;
	}
	dirty |= ui::input("Value", node.comparison_value);
	return dirty;
}

bool create_variable_node_update_editor(create_variable_node& node) {
	bool dirty = ui::checkbox("Global", node.is_global);
	dirty |= ui::checkbox("Persistent", node.new_variable.persistent);
	dirty |= ui::checkbox("Overwrite", node.overwrite);
	if (auto index = ui::combo("Type", { "string", "int", "bool", "float" }, static_cast<int>(node.new_variable.type))) {
		node.new_variable.type = static_cast<variable_type>(index.value());
		switch (node.new_variable.type) {
		case variable_type::string:
			node.new_variable.value = "";
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
	dirty |= ui::input("Name", node.new_variable.name);
	if (node.new_variable.type == variable_type::string) {
		dirty |= ui::input("Value##string", std::get<std::string>(node.new_variable.value));
	} else if (node.new_variable.type == variable_type::integer) {
		dirty |= ui::input("Value##int", std::get<int>(node.new_variable.value));
	} else if (node.new_variable.type == variable_type::floating) {
		dirty |= ui::input("Value##float", std::get<float>(node.new_variable.value));
	} else if (node.new_variable.type == variable_type::boolean) {
		bool as_bool = (std::get<int>(node.new_variable.value) != 0);
		dirty |= ui::checkbox("Value##bool", as_bool);
		node.new_variable.value = as_bool ? 1 : 0;
	}
	return dirty;
}

bool delete_variable_node_update_editor(delete_variable_node& node) {
	bool dirty = ui::checkbox("Global", node.is_global);
	dirty |= ui::input("Name", node.variable_name);
	return dirty;
}

bool execute_script_node_update_editor(execute_script_node& node) {
	bool dirty = ui::input("Script", node.script_id);
	return dirty;
}

bool message_node_update_editor(message_node& node) {
	return ui::input("##message", node.text, { 360.0f, 160.0f });
}

bool modify_variable_node_update_editor(modify_variable_node& node) {
	bool dirty = ui::checkbox("Global", node.is_global);
	dirty |= ui::input("Name", node.variable_name);
	if (auto new_operator = ui::combo("Operator", { "= set", "+- negate", "+ add", "* multiply", "/ divide" }, static_cast<int>(node.modify_operator))) {
		node.modify_operator = static_cast<variable_operator>(new_operator.value());
		dirty = true;
	}
	if (auto new_type = ui::combo("##other-type", { "value", "local", "global" }, static_cast<int>(node.other_type))) {
		node.other_type = static_cast<other_variable_type>(new_type.value());
		dirty = true;
	}
	dirty |= ui::input("##set-value", node.modify_value);
	return dirty;
}

bool random_condition_node_update_editor(random_condition_node& node) {
	if (auto percent = node.chance * 100.0f; ui::input("% chance of success", percent)) {
		percent = clamp(percent, 0.0f, 100.0f);
		node.chance = percent / 100.0f;
		return true;
	} else {
		return false;
	}
}

bool random_output_node_update_editor(random_output_node& node) {
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
	dirty |= ui::input("##position", node.position);
	return dirty;
}

bool trigger_event_node_update_editor(trigger_event_node& node) {
	bool dirty = ui::input("##event-id", node.event_id);
	return dirty;
}

bool variable_exists_node_update_editor(variable_exists_node& node) {
	bool dirty = ui::checkbox("Global", node.is_global);
	dirty |= ui::input("Name", node.variable_name);
	return dirty;
}

}
