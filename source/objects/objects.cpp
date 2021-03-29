#include "objects/objects.hpp"
#include "graphics/ui.hpp"
#include "random.hpp"
#include "log.hpp"

namespace nfwk {

std::u8string normalized_identifier(std::u8string_view id) {
	std::u8string normalized_id{ id };
	for (auto& character : normalized_id) {
		if (!std::isalnum(character)) {
			character = '-';
		}
	}
	return normalized_id;
}

bool is_identifier_normalized(std::u8string_view id) {
	return normalized_identifier(id) == id;
}

script_event::script_event(script_event_container& container, io_stream& stream) : container{ &container } {
	read(stream);
}

script_event::script_event(script_event_container& container, std::u8string_view id) : container{ &container }, id{ id } {
	ASSERT(is_identifier_normalized(id));
}

std::u8string script_event::get_id() const {
	return container ? container->get_id() + u8":" + id : id;
}

void object_class::attach_script(const std::u8string& script_id) {
	scripts.push_back(script_id);
}

void object_class::detach_script(const std::u8string& script_id) {
	if (const auto it = std::find(scripts.begin(), scripts.end(), script_id); it != scripts.end()) {
		scripts.erase(it);
	}
}

bool object_class::supports_event(const std::u8string& event_id) const {
	return std::find(supported_events.begin(), supported_events.end(), event_id) != supported_events.end();
}

void object_class::attach_event(const std::u8string& event_id) {
	supported_events.push_back(event_id);
}

void object_class::detach_event(const std::u8string& event_id) {
	if (auto it = std::find(supported_events.begin(), supported_events.end(), event_id); it != supported_events.end()) {
		supported_events.erase(it);
	}
}

const std::vector<std::u8string>& object_class::get_supported_events() const {
	return supported_events;
}

void object_instance::write(io_stream& stream) const {
	stream.write(transform);
}

void object_instance::read(io_stream& stream) {
	transform = stream.read<transform2>();
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

std::shared_ptr<object_class> object_manager::find_class(const std::u8string& class_id) {
	for (auto& definition : definitions) {
		if (definition->id == class_id) {
			return definition;
		}
	}
	return nullptr;
}

std::shared_ptr<object_class> object_manager::register_class(const std::u8string& class_id) {
	auto definition = std::make_shared<object_class>();
	definition->id = class_id;
	return definitions.emplace_back(definition);
}

}
