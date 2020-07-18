#include "objects.hpp"
#include "transform.hpp"
#include "ui.hpp"
#include "debug.hpp"
#include "window.hpp"
#include "editor.hpp"

namespace no {

struct object_class_definition {
	std::string id;
	std::string name;
	std::vector<std::string> scripts;
	object_collision collision{ object_collision::none };
};

struct object_instance {
	transform2 transform;
};

struct object_class_instance {
	std::shared_ptr<object_class_definition> definition;
	std::vector<object_instance> instances;
};

class create_object_class_editor : public abstract_editor {
public:

	static constexpr std::string_view title{ "Create object class" };

	void update() override;

	std::string_view get_title() const override {
		return title;
	}

	bool is_dirty() const override {
		return false;
	}

private:

	object_class_definition definition;
	std::optional<int> selected_script;

};

static std::vector<std::shared_ptr<object_class_definition>> definitions;
static std::vector<object_class_instance> instances;

static std::shared_ptr<object_class_definition> find_class_definition(const std::string& class_id) {
	for (auto& definition : definitions) {
		if (definition->id == class_id) {
			return definition;
		}
	}
	return nullptr;
}

void register_object_class(const std::string& class_id) {
	definitions.emplace_back()->id = class_id;
}

void attach_object_class_script(const std::string& class_id, const std::string& script_id) {
	find_class_definition(class_id)->scripts.push_back(script_id);
}

void detach_object_class_script(const std::string& class_id, const std::string& script_id) {
	auto& scripts = find_class_definition(class_id)->scripts;
	if (auto it = std::find(scripts.begin(), scripts.end(), script_id); it != scripts.end()) {
		scripts.erase(it);
	}
}

void set_object_class_collision(const std::string& class_id, object_collision collision) {
	find_class_definition(class_id)->collision = collision;
}

void create_object(const std::string& class_id, const variable_map& variables) {

}

void create_object_class_editor::update() {
	ImGui::PushID("nfwk-create-object-class");
	ImGui::PushItemWidth(256.0f);

	ui::input("Identifier", definition.id);
	ui::input("Name", definition.name);

	if (const auto collision = ui::combo("Collision", { "None", "Extents", "Radius", "Precise" }, static_cast<int>(definition.collision))) {
		definition.collision = static_cast<object_collision>(collision.value());
	}

	ui::separate();
	if (ui::button("Save class")) {
		
	}
	ImGui::PopItemWidth();
	ImGui::PopID();
}

}

namespace no::internal {

void initialize_objects() {
	register_editor<create_object_class_editor>();
}

}
