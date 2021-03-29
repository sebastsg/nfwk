#include "object_editor.hpp"
#include "random.hpp"
#include "graphics/ui.hpp"
#include "utility_functions.hpp"

namespace nfwk {

script_event_editor::script_event_editor(editor_container& container, script_event_container& event_container)
	: abstract_editor{ container }, event_container{ event_container } {}

void script_event_editor::update() {
	std::u8string ui_id{ title };
	ui_id += u8"##";
	ui_id += to_string(reinterpret_cast<std::size_t>(this));
	if (auto _ = ui::window(ui_id, 0, &open)) {
		if (!docked) {
			container.dock(ImGuiDir_None, 0.0f);
			docked = true;
		}
		const auto& events = event_container.get();
		if (auto new_selection = ui::list(u8"##event-list", vector_to_strings(events), selected_event_index, 10)) {
			selected_event_index = new_selection.value();
		}
		if (!events.empty()) {
			ui::inline_next();
			if (ui::button(u8"Delete")) {
				event_container.remove(events[selected_event_index]->get_id());
				if (selected_event_index >= static_cast<int>(events.size())) {
					selected_event_index--;
				}
				dirty = true;
			}
		}
		ui::separate();
		ui::input(u8"New Event", new_event_id);
		new_event_id = normalized_identifier(new_event_id);
		ui::inline_next();
		if (auto _ = ui::disable_if(event_container.exists(new_event_id))) {
			if (ui::button(u8"Add")) {
				event_container.add(new_event_id);
				new_event_id = u8"";
			}
		}
		ui::separate();
		if (auto _ = ui::disable_if(!is_dirty())) {
			if (ui::button(u8"Save")) {
				
			}
		}
	}
}

bool script_event_editor::is_dirty() const {
	return dirty;
}

object_class_editor::object_class_editor(editor_container& container) : abstract_editor{ container } {
	definition.id = random_number_generator::global().string(16);
}

object_class_editor::object_class_editor(editor_container& container, const object_class& definition) : abstract_editor{ container }, definition { definition } {

}

void object_class_editor::update() {
	std::u8string window_id{ title };
	window_id += u8"##";
	window_id += to_string(reinterpret_cast<std::size_t>(this));
	if (auto end = ui::window(window_id, std::nullopt, vector2f{ 560.0f, 800.0f }, ui::default_window_flags, &open)) {
		// Identifier
		if (new_id.has_value()) {
			ui::input(u8"ID", new_id.value());
			if (objects->find_class(definition.id)) {
				ui::inline_next();
				const bool new_id_exists{ objects->find_class(new_id.value()) != nullptr };
				if (auto _ = ui::disable_if(new_id_exists)) {
					if (ui::button(u8"Save##save-new-id")) {
						if (auto existing_definition = objects->find_class(definition.id)) {
							existing_definition->id = new_id.value();
							definition.id = new_id.value();
							new_id = std::nullopt;
						}
					}
				}
			}
		} else {
			ui::text(u8"ID: %s", definition.id.c_str());
			ui::inline_next();
			if (ui::button(u8"Edit")) {
				new_id = definition.id;
			}
		}

		// Name
		dirty |= ui::input(u8"Name", definition.name);

		// Collision
		if (const auto collision = ui::combo(u8"Collision", { u8"None", u8"Extents", u8"Radius", u8"Precise" }, static_cast<int>(definition.collision))) {
			const auto new_collision = static_cast<object_collision>(collision.value());
			dirty |= definition.collision != new_collision;
			definition.collision = new_collision;
		}

		// Variables
		ui::separate();
		ui::colored_text({ 1.0f, 0.9f, 0.5f }, u8"Variables");
		std::vector<std::u8string> variables;
		definition.variables.for_each([&](const variable& variable) {
			variables.push_back(to_string(variable.type) + u8" " + variable.name);
		});
		if (auto new_selected_variable = ui::list(u8"##variable-list", variables, selected_variable, 5)) {
			selected_variable = new_selected_variable.value();
		}
		ImGui::Spacing();
		ImGui::PushItemWidth(200.0f);
		if (const auto type = ui::combo(u8"##new-variable-type", { u8"String", u8"Integer", u8"Boolean", u8"Float" }, static_cast<int>(new_variable_type))) {
			new_variable_type = static_cast<variable_type>(type.value());
		}
		ui::inline_next();
		ui::input(u8"##new-variable-name", new_variable_name);
		new_variable_name = string_to_lowercase(new_variable_name);
		ImGui::PopItemWidth();
		ui::inline_next();
		const bool variable_exists{ definition.variables.find(new_variable_name) != nullptr };
		if (auto _ = ui::disable_if(variable_exists)) {
			if (ui::button(u8"Add##add-new-variable")) {
				variable new_variable;
				new_variable.name = new_variable_name;
				new_variable.type = new_variable_type;
				definition.variables.add(new_variable);
				new_variable_name = u8"";
			}
		}

		// Events
		ui::separate();
		ui::colored_text({ 1.0f, 0.9f, 0.5f }, u8"Events");
		if (auto new_selected_event = ui::list(u8"##event-list", definition.get_supported_events(), selected_event, 5)) {
			selected_event = new_selected_event.value();
		}
		ImGui::Spacing();
		ImGui::PushItemWidth(200.0f);
		ui::input(u8"##new-event-id", new_event_id);
		new_event_id = string_to_lowercase(new_event_id);
		ImGui::PopItemWidth();
		ui::inline_next();
		if (auto _ = ui::disable_if(definition.supports_event(new_event_id))) {
			if (ui::button(u8"Add##add-new-event")) {
				definition.attach_event(new_event_id);
			}
		}

		// Save changes
		ui::separate();
		if (auto _ = ui::disable_if(!dirty)) {
			if (ui::button(u8"Save##save-changes")) {
				save();
			}
		}
	}
}

void object_class_editor::save() {
	if (!objects) {
		error(ui::log, u8"No object manager attached. Can't save.");
		return;
	}
	if (auto existing_definition = objects->find_class(definition.id)) {
		*existing_definition = definition;
		dirty = false;
	} else {
		objects->register_class(definition.id);
		if (auto new_definition = objects->find_class(definition.id)) {
			*new_definition = definition;
			dirty = false;
		}
	}
}

std::u8string_view object_class_editor::get_title() const {
	return title;
}

bool object_class_editor::is_dirty() const {
	return dirty;
}

void object_class_list_editor::update() {
	if (!objects) {
		ui::text(u8"No object manager attached.");
		return;
	}
	std::u8string window_title{ title };
	window_title += u8"##";
	window_title += to_string(reinterpret_cast<std::size_t>(this));
	if (auto _ = ui::window(window_title, vector2f{ 0.0f, 32.0f }, vector2f{ 400.0f, 640.0f }, ui::default_window_flags, &open)) {
		for (const auto& definition : objects->get_classes()) {
			ImGui::PushID(definition->id.c_str());
			if (ui::button(u8"Edit")) {
				container.open(std::make_unique<object_class_editor>(container, *definition));
			}
			ui::inline_next();
			ui::colored_text({ 0.8f, 0.4f, 0.4f }, u8"[%s] %s", definition->id.c_str(), definition->name.c_str());
			ImGui::PopID();
		}
	}
}

}
