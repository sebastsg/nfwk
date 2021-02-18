module;

#include "assert.hpp"

export module nfwk.scripts:object_class;

import std.core;
import std.memory;
import std.filesystem;
import nfwk.core;
import nfwk.assets;
import :variables;
import :events;
import :object_instance;

export namespace nfwk::objects {

enum class object_collision { none, extents, radius, precise };

class object_component {
public:

	virtual ~object_component() = default;

	virtual std::string_view get_name() const = 0;

	virtual void write(io_stream& stream) const = 0;
	virtual void read(io_stream& stream) = 0;

};

class object_sprite_component : public object_component {
public:

	static constexpr std::string_view name{ "Sprite" };

	std::string_view get_name() const override {
		return name;
	}

	void write(io_stream& stream) const override {}
	void read(io_stream& stream) override {}

};

using object_component_constructor = std::function<std::unique_ptr<object_component>()>;

}

namespace nfwk::objects {

std::unordered_map<std::string, object_component_constructor> component_constructors;

}

export namespace nfwk::objects {

void register_object_component(const std::string& name, const object_component_constructor& constructor) {
	component_constructors[name] = constructor;
}

std::unique_ptr<object_component> create_object_component(std::string_view name) {
	return component_constructors[std::string{ name }]();
}

std::vector<std::string> get_object_component_names() {
	return get_map_keys(component_constructors);
}

template<typename Component>
void register_object_component() {
	register_object_component(std::string{ Component::name }, [] {
		return std::make_unique<Component>();
	});
}

template<typename Component>
std::unique_ptr<object_component> create_object_component() {
	return create_object_component(Component::name);
}

class object_class {
public:

	static constexpr std::string_view file_extension{ ".nfwk-object" };

	std::string name;
	object_collision collision{ object_collision::none };
	variable_scope default_variables;
	game_event_container events;
	std::vector<std::unique_ptr<object_component>> components;

	object_class(io_stream& stream) {
		read(stream);
	}

	object_class(int index, std::string_view id) : index{ index }, id{ id }, name{ id }, events{ id } {
		ASSERT(index >= 0);
		ASSERT(is_identifier_normalized(id));
	}

	object_class(const object_class&) = delete;
	object_class(object_class&&) = delete;

	~object_class() = default;

	object_class& operator=(const object_class&) = delete;
	object_class& operator=(object_class&&) = delete;

	[[nodiscard]] std::string to_string() const {
		return id;
	}

	int get_index() const {
		return index;
	}

	std::string_view get_id() const {
		return id;
	}

	std::filesystem::path get_path() const {
		return asset_path("objects/" + id + std::string{ file_extension });
	}

	void change_id(std::string_view new_id) {
		ASSERT(is_identifier_normalized(new_id));
		const auto old_path = get_path();
		id = new_id;
		events.set_id(new_id);
		if (std::filesystem::exists(old_path)) {
			std::filesystem::rename(old_path, get_path());
		}
	}

	object_instance make_instance() const {
		return { index };
	}

	void remake_instance(object_instance& instance) const {
		instance = { index };
	}

	void write(io_stream& stream) const {
		ASSERT(index >= 0);
		ASSERT(is_identifier_normalized(id));
		stream.write(static_cast<std::int32_t>(index));
		stream.write(id);
		stream.write(name);
		stream.write(static_cast<std::int32_t>(collision));
		default_variables.write(stream);
		events.write(stream);
	}

private:

	void read(io_stream& stream) {
		index = stream.read<std::int32_t>();
		id = stream.read<std::string>();
		name = stream.read<std::string>();
		collision = static_cast<object_collision>(stream.read<std::int32_t>());
		default_variables.read(stream);
		events.read(stream);
		ASSERT(index >= 0);
		ASSERT(is_identifier_normalized(id));
	}

	std::string id;
	int index{ 0 };

};

}
