#pragma once

#include "loop.hpp"
#include "transform.hpp"
#include "scripts/variables.hpp"

#include <vector>

namespace nfwk {

// todo: move to core file
[[nodiscard]] std::string to_normalized_identifier(std::string_view id);
[[nodiscard]] bool is_identifier_normalized(std::string_view id);

}

namespace nfwk::script {

enum class object_collision { none, extents, radius, precise };

class script_event_container;

class script_event {
public:

	script_event(script_event_container& container, io_stream& stream);
	script_event(script_event_container& container, std::string_view id);
	script_event(const script_event&) = delete;
	script_event(script_event&&) = default;

	~script_event() = default;

	script_event& operator=(const script_event&) = delete;
	script_event& operator=(script_event&&) = default;

	[[nodiscard]] std::string to_string() const {
		return get_id();
	}

	[[nodiscard]] std::string get_id() const;

	void attach_script(std::string_view script_id);
	void detach_script(std::string_view script_id);
	bool has_script(std::string_view script_id) const;
	[[nodiscard]] const std::vector<std::string>& get_scripts() const;

	void write(io_stream& stream) const;
	void read(io_stream& stream);

private:
	
	script_event_container* container{ nullptr };
	std::string id;
	std::vector<std::string> script_ids;

};

class script_event_container {
public:

	event<std::string_view> on_event;

	script_event_container(io_stream& stream);
	script_event_container() = default;
	script_event_container(const script_event_container&) = delete;
	script_event_container(script_event_container&&) = delete;

	~script_event_container() = default;
	
	script_event_container& operator=(const script_event_container&) = delete;
	script_event_container& operator=(script_event_container&&) = delete;
	
	std::string_view get_id() const;
	void set_id(std::string_view new_id);

	[[nodiscard]] const std::vector<std::shared_ptr<script_event>>& get() const;
	[[nodiscard]] std::vector<std::shared_ptr<script_event>> with_script(std::string_view script_id);

	std::shared_ptr<script_event> add(std::string_view event_id);
	void remove(std::string_view event_id);
	[[nodiscard]] std::shared_ptr<script_event> find(std::string_view event_id);

	void trigger(std::string_view event_id);
	[[nodiscard]] bool exists(std::string_view event_id) const;

	void write(io_stream& stream) const;
	void read(io_stream& stream);

	[[nodiscard]] std::vector<std::shared_ptr<script_event>> search(std::string search_term, int limit);

private:

	std::vector<std::shared_ptr<script_event>> events;
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

	[[nodiscard]] const std::vector<std::string>& get_supported_events() const;
	[[nodiscard]] bool supports_event(const std::string& event_id) const;
	void attach_event(const std::string& event_id);
	void detach_event(const std::string& event_id);

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

};

class object_class_instance {
public:

	void create_instance();

	std::string_view get_class_id() const {
		return definition->id;
	}

private:

	void replace_instance(object_instance& instance);

	std::shared_ptr<object_class> definition;
	std::vector<object_instance> instances;

};

class object_manager {
public:

	std::vector<std::shared_ptr<object_class>> get_classes();
	std::shared_ptr<object_class> find_class(std::string_view class_id);
	std::shared_ptr<object_class> register_class(std::string_view class_id);

	std::vector<std::shared_ptr<object_class_instance>> get_class_instances();
	std::shared_ptr<object_class_instance> find_class_instance(std::string_view class_id);

private:

	std::vector<std::shared_ptr<object_class>> definitions;
	std::vector<std::shared_ptr<object_class_instance>> instances;

};

}
