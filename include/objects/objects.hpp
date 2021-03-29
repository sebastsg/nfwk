#pragma once

#include "loop.hpp"
#include "transform.hpp"
#include "scripts/variables.hpp"

#include <vector>

namespace nfwk {

enum class object_collision { none, extents, radius, precise };

class script_event_container;

[[nodiscard]] std::u8string normalized_identifier(std::u8string_view id);
[[nodiscard]] bool is_identifier_normalized(std::u8string_view id);

class script_event {
public:

	script_event(script_event_container& container, io_stream& stream);
	script_event(script_event_container& container, std::u8string_view id);
	script_event(const script_event&) = delete;
	script_event(script_event&&) = default;

	~script_event() = default;

	script_event& operator=(const script_event&) = delete;
	script_event& operator=(script_event&&) = default;

	std::u8string to_string() const {
		return get_id();
	}

	std::u8string get_id() const;

	void trigger() const {
		on_event.emit();
		for (const auto& script_id : scripts) {
			//run_script(script_id);
		}
	}

	void attach_script(const std::u8string& script_id) {
		ASSERT(!has_script(script_id));
		scripts.push_back(script_id);
	}

	void detach_script(const std::u8string& script_id) {
#ifdef NFWK_CPP_20
		std::erase(scripts, script_id);
#else
		for (std::size_t i{0}; i < scripts.size(); i++) {
			if (scripts[i] == script_id) {
				scripts.erase(scripts.begin() + i);
				break;
			}
		}
#endif
	}

	bool has_script(std::u8string_view script_id) const {
		return std::find(scripts.begin(), scripts.end(), script_id) != scripts.end();
	}

	void write(io_stream& stream) const {
		ASSERT(is_identifier_normalized(id));
		stream.write_string(id);
		stream.write_string_array(scripts);
	}

	void read(io_stream& stream) {
		id = stream.read_string();
		scripts = stream.read_string_array();
		ASSERT(is_identifier_normalized(id));
	}

private:
	
	script_event_container* container{ nullptr };
	std::u8string id;
	std::vector<std::u8string> scripts;
	event<> on_event;

};

class script_event_container {
public:

	script_event_container(io_stream& stream) {
		read(stream);
	}

	script_event_container() = default;

	std::u8string get_id() const {
		return id;
	}

	void set_id(std::u8string_view new_id) {
		id = new_id;
		ASSERT(is_identifier_normalized(id));
	}

	[[nodiscard]] std::vector<script_event*> get() {
		std::vector<script_event*> result;
		for (auto& event : events) {
			result.push_back(&event);
		}
		return result;
	}

	[[nodiscard]] std::vector<script_event*> with_script(std::u8string_view script_id) {
		std::vector<script_event*> result;
		for (auto& event : events) {
			if (event.has_script(script_id)) {
				result.push_back(&event);
			}
		}
		return result;
	}

	script_event* add(std::u8string_view event_id) {
		if (exists(event_id)) {
			return nullptr;
		} else {
			return &events.emplace_back(*this, event_id);
		}
	}

	void remove(std::u8string_view event_id) {
#ifdef NFWK_CPP_20
		std::erase_if(events, [&event_id](const auto& event) {
			return event.get_id() == event_id;
		});
#else
		for (int i{ 0 }; i < static_cast<int>(events.size()); i++) {
			if (events[i].get_id() == event_id) {
				events.erase(events.begin() + i);
				i--;
			}
		}
#endif
	}

	script_event* find(std::u8string_view event_id) {
		auto event = std::find_if(events.begin(), events.end(), [&event_id](const auto& event) {
			return event.get_id() == event_id;
		});
		if (event == events.end()) {
			event = std::find_if(events.begin(), events.end(), [this, &event_id](const auto& event) {
				return event.get_id() == id + u8":" + std::u8string{ event_id };
			});
		}
		return event != events.end() ? &*event : nullptr;
	}

	void trigger(std::u8string_view event_id) {
		if (const auto* event = find(event_id)) {
			event->trigger();
		} else {
			info(scripts::log, u8"{} does not exist.", event_id);
		}
	}

	bool exists(std::u8string_view event_id) const {
		return events.end() != std::find_if(events.begin(), events.end(), [&event_id](const auto& event) {
			return event.get_id() == event_id;
		});
	}

	void write(io_stream& stream) const {
		ASSERT(is_identifier_normalized(id));
		stream.write_string(id);
		stream.write_size<size_length::four_bytes>(events.size());
		for (const auto& event : events) {
			event.write(stream);
		}
	}

	void read(io_stream& stream) {
		events.clear();
		id = stream.read_string();
		const auto count = stream.read_size();
		for (std::size_t i{ 0 }; i < count; i++) {
			events.emplace_back(*this, stream);
		}
		ASSERT(is_identifier_normalized(id));
	}

	[[nodiscard]] std::vector<script_event*> search(const std::u8string& search_term, int limit) {
		std::vector<script_event*> results;
		const auto& search = string_to_lowercase(search_term);
		for (auto& event : events) {
			const auto& event_id = string_to_lowercase(event.get_id());
			if (event_id.find(search) != std::u8string::npos) {
				results.push_back(&event);
			}
			if (static_cast<int>(results.size()) >= limit) {
				break;
			}
		}
		return results;
	}

private:

	std::vector<script_event> events;
	std::u8string id;

};

class object_class {
public:

	std::u8string id;
	std::u8string name;
	object_collision collision{ object_collision::none };
	variable_scope variables;

	void attach_script(const std::u8string& script_id);
	void detach_script(const std::u8string& script_id);

	bool supports_event(const std::u8string& event_id) const;
	void attach_event(const std::u8string& event_id);
	void detach_event(const std::u8string& event_id);
	const std::vector<std::u8string>& get_supported_events() const;

private:

	std::vector<std::u8string> scripts;
	std::vector<std::u8string> supported_events;

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

private:

	void replace_instance(object_instance& instance);

	std::shared_ptr<object_class> definition;
	std::vector<object_instance> instances;

};

class object_manager {
public:

	std::vector<std::shared_ptr<object_class>> get_classes();
	std::shared_ptr<object_class> find_class(const std::u8string& class_id);
	std::shared_ptr<object_class> register_class(const std::u8string& class_id);

private:

	std::vector<std::shared_ptr<object_class>> definitions;
	std::vector<object_class_instance> instances;

};

}
