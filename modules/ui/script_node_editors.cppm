export module nfwk.ui:script_node_editors;

import std.core;
import nfwk.core;
import nfwk.scripts;
import :imgui_wrapper;

export namespace nfwk {

#if 0
bool choice_node_update_editor() {
	return ui::input("##choice", text, { 360.0f, 50.0f });
}

bool compare_variable_node_update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::input("Name", variable_name);
	if (auto new_comparison = ui::combo("Comparison", { "==", "!=", ">", "<", ">=", "<=" }, static_cast<int>(comparison_operator))) {
		comparison_operator = static_cast<variable_comparison>(new_comparison.value());
		dirty = true;
	}
	if (auto new_type = ui::combo("##other-type", { "Value", "Local", "Global" }, static_cast<int>(other_type))) {
		other_type = static_cast<other_variable_type>(new_type.value());
		dirty = true;
	}
	dirty |= ui::input("Value", comparison_value);
	return dirty;
}

bool create_variable_node_update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::checkbox("Persistent", new_variable.persistent);
	dirty |= ui::checkbox("Overwrite", overwrite);
	if (auto index = ui::combo("Type", { "String", "Integer", "Boolean", "Float" }, static_cast<int>(new_variable.type))) {
		new_variable.type = static_cast<variable_type>(index.value());
		switch (new_variable.type) {
		case variable_type::string:
			new_variable.value = "";
			break;
		case variable_type::integer:
		case variable_type::boolean:
			new_variable.value = 0;
			break;
		case variable_type::floating:
			new_variable.value = 0.0f;
			break;
		}
		dirty = true;
	}
	dirty |= ui::input("Name", new_variable.name);
	if (new_variable.type == variable_type::string) {
		dirty |= ui::input("Value##string", std::get<std::string>(new_variable.value));
	} else if (new_variable.type == variable_type::integer) {
		dirty |= ui::input("Value##integer", std::get<int>(new_variable.value));
	} else if (new_variable.type == variable_type::floating) {
		dirty |= ui::input("Value##float", std::get<float>(new_variable.value));
	} else if (new_variable.type == variable_type::boolean) {
		bool as_bool = (std::get<int>(new_variable.value) != 0);
		dirty |= ui::checkbox("Value##bool", as_bool);
		new_variable.value = as_bool ? 1 : 0;
	}
	return dirty;
}

bool delete_variable_node_update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::input("Name", variable_name);
	return dirty;
}

bool execute_script_node_update_editor() {
	bool dirty = ui::input("Script", script_id);
	return dirty;
}

bool message_node_update_editor() {
	return ui::input("##message", text, { 360.0f, 160.0f });
}

bool modify_variable_node_update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::input("Name", variable_name);
	if (auto new_operator = ui::combo("Operator", { "Set", "Negate", "Add", "Multiply", "Divide" }, static_cast<int>(modify_operator))) {
		modify_operator = static_cast<variable_operator>(new_operator.value());
		dirty = true;
	}
	if (auto new_type = ui::combo("##other-type", { "Value", "Local", "Global" }, static_cast<int>(other_type))) {
		other_type = static_cast<other_variable_type>(new_type.value());
		dirty = true;
	}
	dirty |= ui::input("##set-value", modify_value);
	return dirty;
}

bool random_condition_node_update_editor() {
	bool dirty = ui::input("% Chance of success", percent);
	percent = clamp(percent, 0, 100);
	return dirty;
}

bool random_output_node_update_editor() {
	return false;
}

bool spawn_object_node_update_editor() {
	bool dirty{ false };
	auto selected_result = class_search.update([&](std::string search_term, int limit) {
		return objects::search_class(normalized_identifier(search_term), limit);
	});
	if (selected_result.has_value()) {
		selected_class_index = selected_result.value()->get_index();
	}
	dirty |= ui::input("##position", position);
	return dirty;
}

bool trigger_event_node_update_editor() {
	bool dirty = ui::input("##event-id", event_id);
	if (!game_event_container::global().exists(event_id)) {
		ui::colored_text({ 1.0f, 0.4f, 0.4f }, "Event does not exist.");
	}
	return dirty;
}

bool variable_exists_node_update_editor() {
	bool dirty = ui::checkbox("Global", is_global);
	dirty |= ui::input("Name", variable_name);
	return dirty;
}
#endif
}
