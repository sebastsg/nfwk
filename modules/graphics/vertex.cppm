export module nfwk.graphics:vertex;

import std.core;
import nfwk.core;

export namespace nfwk {

template<typename Vertex>
vector3f find_min_vertex(const std::vector<Vertex>& vertices) {
	vector3f min{ std::numeric_limits<float>::max() };
	for (const auto& vertex : vertices) {
		min.x = std::min(min.x, vertex.position.x);
		min.y = std::min(min.y, vertex.position.y);
		min.z = std::min(min.z, vertex.position.z);
	}
	return min;
}

template<typename Vertex>
vector3f find_max_vertex(const std::vector<Vertex>& vertices) {
	vector3f max{ -std::numeric_limits<float>::max() };
	for (const auto& vertex : vertices) {
		max.x = std::max(max.x, vertex.position.x);
		max.y = std::max(max.y, vertex.position.y);
		max.z = std::max(max.z, vertex.position.z);
	}
	return max;
}

// todo: make this an "int_to_float" and "byte_to_int" etc. type enum instead?
enum class attribute_component { is_float, is_integer, is_byte };

struct vertex_attribute_specification {

	attribute_component type{ attribute_component::is_float };
	int components{ 0 };
	bool normalized{ false };

	constexpr vertex_attribute_specification(int components) 
		: components(components) {}

	constexpr vertex_attribute_specification(attribute_component type, int components)
		: type(type), components(components) {}

	constexpr vertex_attribute_specification(attribute_component type, int components, bool normalized)
		: type(type), components(components), normalized(normalized) {}

};

using vertex_specification = std::vector<vertex_attribute_specification>;

struct tiny_sprite_vertex {
	static constexpr vertex_attribute_specification attributes[]{ 2, 2 };
	vector2f position;
	vector2f tex_coords;
};

struct sprite_vertex {
	static constexpr vertex_attribute_specification attributes[]{ 2, 4, 2 };
	vector2f position;
	vector4f color{ 1.0f };
	vector2f tex_coords;
};

struct static_mesh_vertex {
	static constexpr vertex_attribute_specification attributes[]{ 3, 3, 2 };
	vector3f position;
	vector3f color{ 1.0f };
	vector2f tex_coords;
};

struct animated_mesh_vertex {
	static constexpr vertex_attribute_specification attributes[]{
		3, 4, 2, 3, 3, 3, 4, 2, { attribute_component::is_integer, 4 }, { attribute_component::is_integer, 2 }
	};
	vector3f position;
	vector4f color{ 1.0f };
	vector2f tex_coords;
	vector3f normal;
	vector3f tangent;
	vector3f bitangent;
	vector4f weights;
	vector2f weights_extra;
	vector4i bones;
	vector2i bones_extra;
};

struct pick_vertex {
	static constexpr vertex_attribute_specification attributes[]{ 3, 3 };
	vector3f position;
	vector3f color;
};

}
