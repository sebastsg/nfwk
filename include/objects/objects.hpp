#pragma once

#include "loop.hpp"
#include "transform.hpp"
#include "scripts/variables.hpp"

#include <functional>
#include <vector>

namespace no::internal {
void initialize_objects();
}

namespace no {

enum class object_collision { none, extents, radius, precise };

class game_event {
public:

	std::string id;
	
};

class object_class {
public:

	std::string id;
	std::string name;
	object_collision collision{ object_collision::none };
	variable_scope variables;

	void attach_script(const std::string& script_id);
	void detach_script(const std::string& script_id);

	bool supports_event(const std::string& event_id) const;
	void attach_event(const std::string& event_id);
	void detach_event(const std::string& event_id);
	const std::vector<std::string>& get_supported_events() const;

private:

	std::vector<std::string> scripts;
	std::vector<std::string> supported_events;

};

class object_instance {
public:

	bool alive{ true };

	transform2 transform;

	void write(io_stream& stream) const;
	void read(io_stream& stream);

private:

};

class object_class_instance {
public:

	void create_instance();

private:

	void replace_instance(object_instance& instance);

	std::shared_ptr<object_class> definition;
	std::vector<object_instance> instances;

};

class object_manager {
public:

	std::vector<std::shared_ptr<object_class>> get_classes();
	std::shared_ptr<object_class> find_class(const std::string& class_id);
	std::shared_ptr<object_class> register_class(const std::string& class_id);

private:

	std::vector<std::shared_ptr<object_class>> definitions;
	std::vector<object_class_instance> instances;

};

object_manager& get_object_manager();

}
