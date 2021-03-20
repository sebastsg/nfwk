#include "object_editor.hpp"
#include "random.hpp"
#include "graphics/ui.hpp"

namespace nfwk {

object_class_editor::object_class_editor(editor_state& editor) : abstract_editor{ editor } {
	definition.id = random_number_generator::global().string(16);
}

object_class_editor::object_class_editor(editor_state& editor, const object_class& definition) : abstract_editor{ editor }, definition { definition } {

}

void object_class_editor::update() {
	auto& objects = get_object_manager();
	if (auto end = ui::push_window(STRING(title << "##" << this), std::nullopt, vector2f{ 560.0f, 800.0f }, ui::default_window_flags, &open)) {
		
		// Identifier
		if (new_id.has_value()) {
			ui::input("ID", new_id.value());
			if (objects.find_class(definition.id)) {
				ui::inline_next();
				const bool new_id_exists{ objects.find_class(new_id.value()) != nullptr };
				if (new_id_exists) {
					ui::begin_disabled();
				}
				if (ui::button("Save##save-new-id")) {
					if (auto existing_definition = objects.find_class(definition.id)) {
						existing_definition->id = new_id.value();
						definition.id = new_id.value();
						new_id = std::nullopt;
					}
				}
				if (new_id_exists) {
					ui::end_disabled();
				}
			}
		} else {
			ui::text("ID: %s", definition.id.c_str());
			ui::inline_next();
			if (ui::button("Edit")) {
				new_id = definition.id;
			}
		}

		// Name
		dirty |= ui::input("Name", definition.name);

		// Collision
		if (const auto collision = ui::combo("Collision", { "None", "Extents", "Radius", "Precise" }, static_cast<int>(definition.collision))) {
			const auto new_collision = static_cast<object_collision>(collision.value());
			dirty |= definition.collision != new_collision;
			definition.collision = new_collision;
		}

		// Variables
		ui::separate();
		ui::colored_text({ 1.0f, 0.9f, 0.5f }, "Variables");
		std::vector<std::string> variables;
		definition.variables.for_each([&](const variable& variable) {
			variables.push_back(STRING(variable.type << " " << variable.name));
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
		const bool variable_exists{ definition.variables.find(new_variable_name).has_value() };
		if (variable_exists) {
			ui::begin_disabled();
		}
		if (ui::button("Add##add-new-variable")) {
			variable new_variable;
			new_variable.name = new_variable_name;
			new_variable.type = new_variable_type;
			definition.variables.add(new_variable);
			new_variable_name = "";
		}
		if (variable_exists) {
			ui::end_disabled();
		}

		// Events
		ui::separate();
		ui::colored_text({ 1.0f, 0.9f, 0.5f }, "Events");
		if (auto new_selected_event = ui::list("##event-list", definition.get_supported_events(), selected_event, 5)) {
			selected_event = new_selected_event.value();
		}
		ImGui::Spacing();
		ImGui::PushItemWidth(200.0f);
		ui::input("##new-event-id", new_event_id);
		new_event_id = string_to_lowercase(new_event_id);
		ImGui::PopItemWidth();
		ui::inline_next();
		const bool event_exists{ definition.supports_event(new_event_id) };
		if (event_exists) {
			ui::begin_disabled();
		}
		if (ui::button("Add##add-new-event")) {
			definition.attach_event(new_event_id);
		}
		if (event_exists) {
			ui::end_disabled();
		}

		// Save changes
		ui::separate();
		const bool was_dirty{ dirty }; // save will set to false, so we must store it here.
		if (!was_dirty) {
			ui::begin_disabled();
		}
		if (ui::button("Save##save-changes")) {
			save();
		}
		if (!was_dirty) {
			ui::end_disabled();
		}
	}
}

void object_class_editor::save() {
	auto& objects = get_object_manager();
	if (auto existing_definition = objects.find_class(definition.id)) {
		*existing_definition = definition;
		dirty = false;
	} else {
		objects.register_class(definition.id);
		if (auto new_definition = objects.find_class(definition.id)) {
			*new_definition = definition;
			dirty = false;
		}
	}
}

std::string_view object_class_editor::get_title() const {
	return title;
}

bool object_class_editor::is_dirty() const {
	return dirty;
}

void object_class_list_editor::update() {
	auto& objects = get_object_manager();
	if (auto end = ui::push_window(STRING(title << "##" << this), vector2f{ 0.0f, 32.0f }, vector2f{ 400.0f, 640.0f }, ui::default_window_flags, &open)) {
		for (const auto& definition : objects.get_classes()) {
			ImGui::PushID(definition->id.c_str());
			if (ui::button("Edit")) {
				editor.open(std::make_unique<object_class_editor>(editor, *definition));
			}
			ui::inline_next();
			ui::colored_text({ 0.8f, 0.4f, 0.4f }, "[%s] %s", definition->id.c_str(), definition->name.c_str());
			ImGui::PopID();
		}
	}
}

}
