module;

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

export module nfwk.ui:object_class_editor;

import std.core;
import nfwk.core;
import nfwk.scripts;
import :editor;
import :imgui_wrapper;

export namespace nfwk::objects {

class object_class_editor : public abstract_editor {
public:

	object_class_editor(int class_index) : class_index{ class_index } {
		opened_classes.push_back(class_index);
	}

	~object_class_editor() {
		std::erase(opened_classes, class_index);
	}

	bool is_dirty() const override {
		return dirty;
	}

	void update() override {
		if (const auto* editing_class = get_class(class_index)) {
			// TODO:
			// editing_class->name + "###" + this
			if (auto end = ui::window(editing_class->name, 0, &open)) {
				if (auto editor = static_cast<editor_state*>(program_state::current()); editor && !docked) {
					editor->dock(ImGuiDir_None, 0.0f);
					docked = true;
				}
				update_edit();
			}
		}
	}

	static bool is_open(int class_index) {
		return std::find(opened_classes.begin(), opened_classes.end(), class_index) != opened_classes.end();
	}

private:

	void update_edit() {
		if (auto editing_class = get_class(class_index)) {
			update_general(*editing_class);
			update_variables(*editing_class);
			update_events(*editing_class);

			ImGui::PushItemWidth(400.0f);
			ui::separate();
			const auto& component_names = get_object_component_names();
			if (!component_names.empty()) {
				ui::combo("##new-component", component_names, component_to_add);
			}
			ui::inline_next();
			// todo: should not be possible to add duplicate components.
			if (auto disabled = ui::disable_if(component_to_add < 0 || component_to_add >= static_cast<int>(component_names.size()))) {
				if (ui::button("Add Component")) {
					editing_class->components.emplace_back(create_object_component(component_names[component_to_add]));
				}
			}
			ImGui::PopItemWidth();
			ui::separate();

			for (auto& component : editing_class->components) {
				//dirty |= component->update_editor();
			}
			update_save();
		}
	}

	void update_change_id() {
		if (new_id.has_value()) {
			ui::input("ID", new_id.value());
			ui::inline_next();
			if (auto disabled = ui::disable_if(find_class(new_id.value()).has_value())) {
				if (ui::button("Save##save-new-id")) {
					if (set_class_id(class_index, new_id.value())) {
						new_id = std::nullopt;
						save_class(class_index);
					}
				}
			}
		} else {
			const auto* editing_class = get_class(class_index);
			ui::text("ID: %s", editing_class->get_id().data());
			ui::inline_next();
			if (ui::button("Edit")) {
				new_id = editing_class->get_id();
			}
		}
	}

	void update_general(object_class& editing_class) {
		update_change_id();
		ImGui::PushItemWidth(320.0f);
		dirty |= ui::input("Name", editing_class.name);
		ui::inline_next();
		ImGui::Dummy({ 64.0f, 0.0f });
		ui::inline_next();
		if (const auto index = ui::combo("Collision", { "None", "Extents", "Radius", "Precise" }, static_cast<int>(editing_class.collision))) {
			editing_class.collision = static_cast<object_collision>(index.value());
			dirty = true;
		}
		ImGui::PopItemWidth();
	}

	void update_variables(object_class& editing_class) {
		ui::separate();
		ui::colored_text({ 1.0f, 0.9f, 0.5f }, "Variables");
		std::vector<std::string> variables;
		editing_class.default_variables.for_each([&](const variable& variable) {
			//variables.push_back(variable.type + " " + variable.name);
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
		dirty |= ui::input("##new-variable-name", new_variable_name);
		new_variable_name = string_to_lowercase(new_variable_name);
		ImGui::PopItemWidth();
		ui::inline_next();
		if (auto disabled = ui::disable_if(editing_class.default_variables.find(new_variable_name).has_value())) {
			if (ui::button("Add##add-new-variable")) {
				variable new_variable;
				new_variable.name = new_variable_name;
				new_variable.type = new_variable_type;
				new_variable.value = new_variable.default_value();
				editing_class.default_variables.add(new_variable, false);
				new_variable_name = "";
				dirty = true;
			}
		}
	}

	void update_events(object_class& editing_class) {
		ui::separate();
		ui::colored_text({ 1.0f, 0.9f, 0.5f }, "Events");
		auto events = editing_class.events.get();
		if (auto new_selected_event = ui::list("##event-list", vector_to_strings(events), selected_event, 5)) {
			selected_event = new_selected_event.value();
		}
		if (!events.empty()) {
			ui::inline_next();
			if (ui::button("Delete")) {
				editing_class.events.remove(events[selected_event]->get_id());
				if (selected_event >= static_cast<int>(events.size())) {
					selected_event--;
				}
				dirty = true;
			}
		}
		ImGui::Spacing();
		ImGui::PushItemWidth(200.0f);
		dirty |= ui::input("New Event", new_event_id);
		new_event_id = normalized_identifier(new_event_id);
		new_event_id = string_to_lowercase(new_event_id);
		ImGui::PopItemWidth();
		ui::inline_next();
		if (auto disabled = ui::disable_if(editing_class.events.exists(new_event_id))) {
			if (ui::button("Add")) {
				editing_class.events.add(new_event_id);
				new_event_id = "";
				dirty = true;
			}
		}
	}

	void update_save() {
		ui::separate();
		if (auto disabled = ui::disable_if(!dirty)) {
			if (ui::button("Save")) {
				save_class(class_index);
				dirty = false;
			}
		}
	}

	int class_index{ 0 };
	std::optional<std::string> new_id;
	bool dirty{ false };

	std::string new_variable_name;
	variable_type new_variable_type{ variable_type::string };
	int selected_variable{ 0 };

	std::string new_event_id;
	int selected_event{ 0 };

	int component_to_add{ 0 };

	bool docked{ false };

	static std::vector<int> opened_classes;

};

std::vector<int> object_class_editor::opened_classes;

class object_class_list_editor : public abstract_editor {
public:

	static constexpr std::string_view title{ "Objects" };

	void update() override {
		// TODO: title + "##" + this
		if (auto end = ui::window(title, 0, &open)) {
			if (auto editor = static_cast<editor_state*>(program_state::current()); editor && !docked) {
				editor->dock(ImGuiDir_Left, 0.2f);
				docked = true;
			}
			if (ui::button("Create new class")) {
				if (const auto class_index = register_class(normalized_identifier(random_number_generator::global().string(16)))) {
					if (auto editor = static_cast<editor_state*>(program_state::current())) {
						editor->open(std::make_unique<object_class_editor>(class_index.value()));
					}
				}
			}
			ui::separate();
			for (const auto& class_ : get_classes()) {
				if (auto disabled = ui::disable_if(object_class_editor::is_open(class_->get_index()))) {
					/*if (ui::button("[" + class_->get_id() + "] " + class_->name)) {
						if (auto editor = static_cast<editor_state*>(program_state::current())) {
							editor->open(std::make_unique<object_class_editor>(class_->get_index()));
						}
					}*/
				}
			}
		}
	}

	bool is_dirty() const override {
		return false;
	}
	
private:

	bool docked{ false };

};

}
