#pragma once

#include "loop.hpp"
#include "math.hpp"

#include <functional>
#include <vector>

namespace no {

namespace internal {
void initialize_objects();
}

class variable_map;

enum class object_collision { none, extents, radius, precise };

void register_object_class(const std::string& class_id);
void attach_object_class_script(const std::string& class_id, const std::string& script_id);
void detach_object_class_script(const std::string& class_id, const std::string& script_id);
void set_object_class_collision(const std::string& class_id, object_collision collision);

void create_object(const std::string& class_id, const variable_map& variables);

}
