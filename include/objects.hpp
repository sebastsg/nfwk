#pragma once

#include "loop.hpp"
#include "transform.hpp"

#include <functional>
#include <vector>

namespace no::internal {
void initialize_objects();
}

namespace no {

class variable_map;

enum class object_collision { none, extents, radius, precise };

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

std::vector<std::shared_ptr<object_class_definition>> get_object_class_definitions();
std::shared_ptr<object_class_definition> find_class_definition(const std::string& class_id);
void register_object_class(const std::string& class_id);
void attach_object_class_script(const std::string& class_id, const std::string& script_id);
void detach_object_class_script(const std::string& class_id, const std::string& script_id);
void set_object_class_collision(const std::string& class_id, object_collision collision);

void create_object(const std::string& class_id, const variable_map& variables);

}
