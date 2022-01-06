#include "object_editor.hpp"
#include "random.hpp"
#include "graphics/ui.hpp"
#include "utility_functions.hpp"

namespace nfwk::script {

script_event_editor::script_event_editor(script_event_container& event_container) : event_container{ event_container } {}

void script_event_editor::update(ui::window_container& container) {
	if (!docked) {
		container.dock(ui::docking_direction::none, 0.0f);
		docked = true;
	}
	const auto& events = event_container.get();
	if (auto new_selection = ui::list("##event-list", to_strings(events), selected_event_index, 10)) {
		selected_event_index = new_selection.value();
	}
	if (!events.empty()) {
		ui::inline_next();
		if (ui::button("Delete")) {
			event_container.remove(events[selected_event_index]->get_id());
			if (selected_event_index >= static_cast<int>(events.size())) {
				selected_event_index--;
			}
			dirty = true;
		}
	}
	ui::separate();
	ui::input("New Event", new_event_id);
	new_event_id = to_normalized_identifier(new_event_id);
	ui::inline_next();
	if (auto _ = ui::disable_if(event_container.exists(new_event_id))) {
		if (ui::button("Add")) {
			event_container.add(new_event_id);
			new_event_id = "";
		}
	}
	ui::separate();
	if (auto _ = ui::disable_if(!has_unsaved_changes())) {
		if (ui::button("Save")) {

		}
	}
}

bool script_event_editor::has_unsaved_changes() const {
	return dirty;
}

std::string script_event_editor::get_name() const {
	return "Script events";
}

object_class_editor::object_class_editor() {
	definition->id = random_number_generator::any().string(16);
}

object_class_editor::object_class_editor(std::shared_ptr<object_class> definition) : definition{ std::move(definition) } {}

void object_class_editor::update(ui::window_container& container) {
	// Identifier
	if (new_id.has_value()) {
		ui::input("ID", new_id.value());
		if (objects->find_class(definition->id)) {
			ui::inline_next();
			const bool new_id_exists{ objects->find_class(new_id.value()) != nullptr };
			if (auto _ = ui::disable_if(new_id_exists)) {
				if (ui::button("Save##save-new-id")) {
					if (auto existing_definition = objects->find_class(definition->id)) {
						existing_definition->id = new_id.value();
						definition->id = new_id.value();
						new_id = std::nullopt;
					}
				}
			}
		}
	} else {
		ui::text("ID: %s", definition->id.c_str());
		ui::inline_next();
		if (ui::button("Edit")) {
			new_id = definition->id;
		}
	}

	// Name
	dirty |= ui::input("Name", definition->name);

	// Collision
	if (const auto collision = ui::combo("Collision", { "None", "Extents", "Radius", "Precise" }, static_cast<int>(definition->collision))) {
		const auto new_collision = static_cast<object_collision>(collision.value());
		dirty |= definition->collision != new_collision;
		definition->collision = new_collision;
	}

	// Variables
	ui::separate();
	ui::colored_text({ 1.0f, 0.9f, 0.5f }, "Variables");
	std::vector<std::string> variables;
	definition->variables.for_each([&](const variable& variable) {
		variables.push_back(enum_string(variable.type) + " " + variable.name);
	});
	if (auto new_selected_variable = ui::list("##variable-list", variables, selected_variable, 5)) {
		selected_variable = new_selected_variable.value();
	}
	ImGui::Spacing();
	ImGui::PushItemWidth(200.0f);
	if (const auto type = ui::combo("##new-variable-type", { "String", "Integer", "Boolean", "Float" }, static_cast<int>(new_variable_type))) {
		new_variable_type = static_cast<variable_type>(type.value());
	}
	ui::inline_next();
	ui::input("##new-variable-name", new_variable_name);
	new_variable_name = string_to_lowercase(new_variable_name);
	ImGui::PopItemWidth();
	ui::inline_next();
	const bool variable_exists{ definition->variables.find(new_variable_name) != nullptr };
	if (auto _ = ui::disable_if(variable_exists)) {
		if (ui::button("Add##add-new-variable")) {
			variable new_variable;
			new_variable.name = new_variable_name;
			new_variable.type = new_variable_type;
			definition->variables.add(new_variable);
			new_variable_name = "";
		}
	}

	// Events
	ui::separate();
	ui::colored_text({ 1.0f, 0.9f, 0.5f }, "Events");
	if (auto new_selected_event = ui::list("##event-list", definition->get_supported_events(), selected_event, 5)) {
		selected_event = new_selected_event.value();
	}
	ImGui::Spacing();
	ImGui::PushItemWidth(200.0f);
	ui::input("##new-event-id", new_event_id);
	new_event_id = string_to_lowercase(new_event_id);
	ImGui::PopItemWidth();
	ui::inline_next();
	if (auto _ = ui::disable_if(definition->supports_event(new_event_id))) {
		if (ui::button("Add##add-new-event")) {
			definition->attach_event(new_event_id);
		}
	}

	// Save changes
	ui::separate();
	if (auto _ = ui::disable_if(!dirty)) {
		if (ui::button("Save##save-changes")) {
			save();
		}
	}
}

void object_class_editor::save() {
	if (!objects) {
		error(ui::log, "No object manager attached. Can't save.");
		return;
	}
	if (!objects->find_class(definition->id)) {
		objects->register_class(definition->id);
		if (auto new_definition = objects->find_class(definition->id)) {
			new_definition = definition;
			dirty = false;
		}
	}
}

bool object_class_editor::has_unsaved_changes() const {
	return dirty;
}

std::string object_class_editor::get_name() const {
	return "Object class";
}

void object_class_list_editor::update(ui::window_container& container) {
	if (!objects) {
		ui::text("No object manager attached.");
		return;
	}
	for (const auto& definition : objects->get_classes()) {
		ImGui::PushID(definition->id.c_str());
		if (ui::button("Edit")) {
			container.open(std::make_unique<object_class_editor>(definition));
		}
		ui::inline_next();
		ui::colored_text({ 0.8f, 0.4f, 0.4f }, "[%s] %s", definition->id.c_str(), definition->name.c_str());
		ImGui::PopID();
	}
}

std::string object_class_list_editor::get_name() const {
	return "Object classes";
}

}
