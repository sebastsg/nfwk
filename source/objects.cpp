#include "objects.hpp"
#include "transform.hpp"
#include "ui.hpp"

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

namespace ui {

void create_object_class() {
	static thread_local object_class_definition definition;
	text("Create object class");
	input("Identifier", definition.id);
	input("Name", definition.name);
	if (const auto collision = combo("Collision", { "None", "Extents", "Radius", "Precise" }, definition.collision)) {
		definition.collision = collision.value();
	}
	if (button("Save")) {

	}
}

}

}
