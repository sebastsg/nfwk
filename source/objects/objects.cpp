#include "objects/objects.hpp"
#include "graphics/ui.hpp"
#include "random.hpp"
#include "log.hpp"
#include "utility_functions.hpp"

namespace nfwk {

std::string to_normalized_identifier(std::string_view id) {
	std::string normalized_id{ id };
	for (auto& character : normalized_id) {
		if (!std::isalnum(character)) {
			character = '-';
		}
	}
	return normalized_id;
}

bool is_identifier_normalized(std::string_view id) {
	return to_normalized_identifier(id) == id;
}

}

namespace nfwk::script {

script_event::script_event(script_event_container& container, io_stream& stream) : container{ &container } {
	read(stream);
}

script_event::script_event(script_event_container& container, std::string_view id) : container{ &container }, id{ id } {
	ASSERT(is_identifier_normalized(id));
}

std::string script_event::get_id() const {
	return container ? std::string{ container->get_id() } + ":" + id : id;
}

void script_event::attach_script(std::string_view script_id) {
	ASSERT(!has_script(script_id));
	script_ids.emplace_back(script_id);
}

void script_event::detach_script(std::string_view script_id) {
#ifdef NFWK_CPP_20
	std::erase(script_ids, script_id);
#else
	for (std::size_t i{ 0 }; i < script_ids.size(); i++) {
		if (script_ids[i] == script_id) {
			script_ids.erase(script_ids.begin() + i);
			break;
		}
	}
#endif
}

const std::vector<std::string>& script_event::get_scripts() const {
	return script_ids;
}

bool script_event::has_script(std::string_view script_id) const {
	return std::find(script_ids.begin(), script_ids.end(), script_id) != script_ids.end();
}

void script_event::write(io_stream& stream) const {
	ASSERT(is_identifier_normalized(id));
	stream.write_string(id);
	stream.write_string_array(script_ids);
}

void script_event::read(io_stream& stream) {
	id = stream.read_string();
	script_ids = stream.read_string_array();
	ASSERT(is_identifier_normalized(id));
}


script_event_container::script_event_container(io_stream& stream) {
	read(stream);
}

std::string_view script_event_container::get_id() const {
	return id;
}

void script_event_container::set_id(std::string_view new_id) {
	id = new_id;
	ASSERT(is_identifier_normalized(id));
}

const std::vector<std::shared_ptr<script_event>>& script_event_container::get() const {
	return events;
}

std::vector<std::shared_ptr<script_event>> script_event_container::with_script(std::string_view script_id) {
	std::vector<std::shared_ptr<script_event>> result;
	for (auto& event : events) {
		if (event->has_script(script_id)) {
			result.push_back(event);
		}
	}
	return result;
}

std::shared_ptr<script_event> script_event_container::add(std::string_view event_id) {
	if (exists(event_id)) {
		return nullptr;
	} else {
		return events.emplace_back(std::make_shared<script_event>(*this, event_id));
	}
}

void script_event_container::remove(std::string_view event_id) {
#ifdef NFWK_CPP_20
	std::erase_if(events, [&event_id](const auto& event) {
		return event->get_id() == event_id;
	});
#else
	for (int i{ 0 }; i < static_cast<int>(events.size()); i++) {
		if (events[i]->get_id() == event_id) {
			events.erase(events.begin() + i);
			i--;
		}
	}
#endif
}

std::shared_ptr<script_event> script_event_container::find(std::string_view event_id) {
	auto it = std::find_if(events.begin(), events.end(), [&event_id](const std::shared_ptr<script_event>& event) {
		return event->get_id() == event_id;
	});
	if (it == events.end()) {
		const auto full_event_id = id + ":" + std::string{ event_id };
		it = std::find_if(events.begin(), events.end(), [&full_event_id](const std::shared_ptr<script_event>& event) {
			return event->get_id() == full_event_id;
		});
	}
	return it != events.end() ? *it : nullptr;
}

void script_event_container::trigger(std::string_view event_id) {
	if (const auto& event = find(event_id)) {
		for (const auto& script_id : event->get_scripts()) {
			on_event.emit(script_id);
		}
	} else {
		info(scripts::log, "{} does not exist.", event_id);
	}
}

bool script_event_container::exists(std::string_view event_id) const {
	return std::find_if(events.begin(), events.end(), [&event_id](const std::shared_ptr<script_event>& event) {
		return event->get_id() == event_id;
	}) != events.end();
}

void script_event_container::write(io_stream& stream) const {
	ASSERT(is_identifier_normalized(id));
	stream.write_string(id);
	stream.write_size<size_length::four_bytes>(events.size());
	for (const auto& event : events) {
		event->write(stream);
	}
}

void script_event_container::read(io_stream& stream) {
	events.clear();
	id = stream.read_string();
	const auto count = stream.read_size();
	for (std::size_t i{ 0 }; i < count; i++) {
		events.emplace_back(std::make_shared<script_event>(*this, stream));
	}
	ASSERT(is_identifier_normalized(id));
}

std::vector<std::shared_ptr<script_event>> script_event_container::search(std::string search_term, int limit) {
	std::vector<std::shared_ptr<script_event>> results;
	const auto& search = string_to_lowercase(std::move(search_term));
	for (auto& event : events) {
		if (const auto& event_id = string_to_lowercase(event->get_id()); event_id.find(search) != std::string::npos) {
			results.push_back(event);
		}
		if (static_cast<int>(results.size()) >= limit) {
			break;
		}
	}
	return results;
}

void object_class::attach_script(const std::string& script_id) {
	scripts.push_back(script_id);
}

void object_class::detach_script(const std::string& script_id) {
#ifdef NFWK_CPP_20
	std::erase(scripts, script_id);
#else
	if (const auto it = std::find(scripts.begin(), scripts.end(), script_id); it != scripts.end()) {
		scripts.erase(it);
	}
#endif
}

const std::vector<std::string>& object_class::get_supported_events() const {
	return supported_events;
}

bool object_class::supports_event(const std::string& event_id) const {
	return std::find(supported_events.begin(), supported_events.end(), event_id) != supported_events.end();
}

void object_class::attach_event(const std::string& event_id) {
	supported_events.push_back(event_id);
}

void object_class::detach_event(const std::string& event_id) {
	if (auto it = std::find(supported_events.begin(), supported_events.end(), event_id); it != supported_events.end()) {
		supported_events.erase(it);
	}
}

void object_instance::write(io_stream& stream) const {
	stream.write_struct(transform);
}

void object_instance::read(io_stream& stream) {
	transform = stream.read_struct<transform2>();
}

void object_class_instance::create_instance() {
	for (auto& instance : instances) {
		if (!instance.alive) {
			replace_instance(instance);
			return;
		}
	}
	replace_instance(instances.emplace_back());
}

void object_class_instance::replace_instance(object_instance& instance) {
	
}

std::vector<std::shared_ptr<object_class>> object_manager::get_classes() {
	return definitions;
}

std::shared_ptr<object_class> object_manager::find_class(std::string_view class_id) {
	for (auto& definition : definitions) {
		if (definition->id == class_id) {
			return definition;
		}
	}
	return nullptr;
}

std::shared_ptr<object_class> object_manager::register_class(std::string_view class_id) {
	auto definition = std::make_shared<object_class>();
	definition->id = class_id;
	return definitions.emplace_back(definition);
}

std::vector<std::shared_ptr<object_class_instance>> object_manager::get_class_instances() {
	return instances;
}

std::shared_ptr<object_class_instance> object_manager::find_class_instance(std::string_view class_id) {
	for (auto& instance : instances) {
		if (instance->get_class_id() == class_id) {
			return instance;
		}
	}
	return nullptr;
}

}
