#include "object_editor.hpp"
#include "random.hpp"
#include "ui.hpp"

namespace no {

object_class_editor::object_class_editor() {
	definition.id = random_number_generator::global().string(16);
}

object_class_editor::object_class_editor(const object_class_definition& definition) : definition{ definition } {

}

void object_class_editor::update() {
	if (auto end = ui::push_window(STRING(title << "##" << this), std::nullopt, vector2f{ 480.0f, 512.0f }, ui::default_window_flags, &open)) {
		
		// Identifier
		if (new_id.has_value()) {
			ui::input("ID", new_id.value());
			ui::inline_next();
			const bool new_id_exists{ find_class_definition(new_id.value()) != nullptr };
			if (new_id_exists) {
				ui::begin_disabled();
			}
			if (ui::button("Save##save-new-id")) {
				if (auto existing_definition = find_class_definition(definition.id)) {
					existing_definition->id = new_id.value();
					definition.id = new_id.value();
					new_id = std::nullopt;
				}
			}
			if (new_id_exists) {
				ui::end_disabled();
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
	if (auto existing_definition = find_class_definition(definition.id)) {
		*existing_definition = definition;
		dirty = false;
	} else {
		register_object_class(definition.id);
		if (auto new_definition = find_class_definition(definition.id)) {
			*new_definition = definition;
			dirty = false;
		}
	}
}

void object_class_list_editor::update() {
	if (auto end = ui::push_window(STRING(title << "##" << this), vector2f{ 0.0f, 32.0f }, vector2f{ 400.0f, 640.0f }, ui::default_window_flags, &open)) {
		for (const auto& definition : get_object_class_definitions()) {
			ImGui::PushID(definition->id.c_str());
			if (ui::button("Edit")) {
				if (auto editor = dynamic_cast<editor_state*>(program_state::current())) {
					editor->open(std::make_unique<object_class_editor>(*definition));
				}
			}
			ui::inline_next();
			ui::colored_text({ 0.8f, 0.4f, 0.4f }, "[%s] %s", definition->id.c_str(), definition->name.c_str());
			ImGui::PopID();
		}
	}
}

}
