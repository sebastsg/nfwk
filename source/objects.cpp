#include "objects.hpp"
#include "object_editor.hpp"
#include "ui.hpp"
#include "debug.hpp"
#include "window.hpp"
#include "random.hpp"

namespace no {

static std::vector<std::shared_ptr<object_class_definition>> definitions;
static std::vector<object_class_instance> instances;

std::vector<std::shared_ptr<object_class_definition>> get_object_class_definitions() {
	return definitions;
}

std::shared_ptr<object_class_definition> find_class_definition(const std::string& class_id) {
	for (auto& definition : definitions) {
		if (definition->id == class_id) {
			return definition;
		}
	}
	return nullptr;
}

void register_object_class(const std::string& class_id) {
	auto definition = std::make_shared<object_class_definition>();
	definition->id = class_id;
	definitions.emplace_back(definition);
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

}

namespace no::internal {

void initialize_objects() {
	register_editor<object_class_editor>();
	register_editor<object_class_list_editor>();
}

}
