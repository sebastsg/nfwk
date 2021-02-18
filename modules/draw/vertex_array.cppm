module;

#include <glew/glew.h>
#include "gl_debug.hpp"

export module nfwk.draw:vertex_array;

import std.core;
import nfwk.core;
import nfwk.graphics;
import :gl_structs;

namespace nfwk {

std::size_t size_of_attribute_component(attribute_component type) {
	switch (type) {
	case attribute_component::is_float: return sizeof(float);
	case attribute_component::is_integer: return sizeof(int);
	case attribute_component::is_byte: return sizeof(std::uint8_t);
	default: return sizeof(float);
	}
}

std::vector<gl::gl_vertex_array> vertex_arrays;

}

export namespace nfwk {

void bind_vertex_array(int id) {
	const auto& vertex_array = vertex_arrays[id];
	CHECK_GL_ERROR(glBindVertexArray(vertex_array.id));
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vertex_array.vertex_buffer.id));
	CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_array.index_buffer.id));
}

int create_vertex_array(const vertex_specification& specification) {
	std::optional<int> id;
	for (std::size_t i{ 0 }; i < vertex_arrays.size(); i++) {
		if (vertex_arrays[i].id == 0) {
			id = static_cast<int>(i);
			break;
		}
	}
	if (!id.has_value()) {
		vertex_arrays.emplace_back();
		id = static_cast<int>(vertex_arrays.size()) - 1;
	}
	auto& vertex_array = vertex_arrays[id.value()];
	CHECK_GL_ERROR(glGenVertexArrays(1, &vertex_array.id));
	CHECK_GL_ERROR(glGenBuffers(1, &vertex_array.vertex_buffer.id));
	CHECK_GL_ERROR(glGenBuffers(1, &vertex_array.index_buffer.id));
	bind_vertex_array(id.value());
	int vertex_size{ 0 };
	for (const auto& attribute : specification) {
		vertex_size += attribute.components * static_cast<int>(size_of_attribute_component(attribute.type));
	}
	char* attribute_pointer{ nullptr };
	for (int i{ 0 }; i < static_cast<int>(specification.size()); i++) {
		const auto& attribute = specification[i];
		const int normalized{ attribute.normalized ? GL_TRUE : GL_FALSE };
		switch (attribute.type) {
		case attribute_component::is_float:
			CHECK_GL_ERROR(glVertexAttribPointer(i, attribute.components, GL_FLOAT, normalized, vertex_size, attribute_pointer));
			break;
		case attribute_component::is_integer:
			CHECK_GL_ERROR(glVertexAttribIPointer(i, attribute.components, GL_INT, vertex_size, attribute_pointer));
			break;
		case attribute_component::is_byte:
			CHECK_GL_ERROR(glVertexAttribPointer(i, attribute.components, GL_UNSIGNED_BYTE, normalized, vertex_size, attribute_pointer));
			break;
		}
		CHECK_GL_ERROR(glEnableVertexAttribArray(i));
		attribute_pointer += static_cast<std::size_t>(attribute.components) * size_of_attribute_component(attribute.type);
	}
	return id.value();
}

void set_vertex_array_vertices(int id, const std::uint8_t* data, std::size_t size) {
	ASSERT(data && size > 0);
	auto& vertex_array = vertex_arrays[id];
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vertex_array.vertex_buffer.id));
	if (vertex_array.vertex_buffer.exists && vertex_array.vertex_buffer.allocated >= size) {
		CHECK_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, size, data));
	} else {
		CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
		vertex_array.vertex_buffer.exists = true;
		vertex_array.vertex_buffer.allocated = size;
	}
}

void set_vertex_array_indices(int id, const std::uint8_t* data, std::size_t size, std::size_t element_size) {
	ASSERT(data && size > 0);
	auto& vertex_array = vertex_arrays[id];
	switch (element_size) {
	case 1:
		vertex_array.index_type = GL_UNSIGNED_BYTE;
		break;
	case 2:
		vertex_array.index_type = GL_UNSIGNED_SHORT;
		break;
	case 4:
		vertex_array.index_type = GL_UNSIGNED_INT;
		break;
	default:
		warning("draw", "Invalid index element size: {}", element_size);
		break;
	}
	CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_array.index_buffer.id));
	if (vertex_array.index_buffer.exists && vertex_array.index_buffer.allocated >= size) {
		CHECK_GL_ERROR(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, data));
	} else {
		CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
		vertex_array.index_buffer.exists = true;
		vertex_array.index_buffer.allocated = size;
	}
	vertex_array.indices = size / element_size;
}

void draw_vertex_array(int id) {
	const auto& vertex_array = vertex_arrays[id];
	CHECK_GL_ERROR(glDrawElements(vertex_array.draw_mode, vertex_array.indices, vertex_array.index_type, nullptr));
}

void draw_vertex_array(int id, std::size_t offset, int count) {
	const auto& vertex_array = vertex_arrays[id];
	CHECK_GL_ERROR(glDrawElements(vertex_array.draw_mode, count, vertex_array.index_type, (void*)offset));
}

void delete_vertex_array(int id) {
	auto& vertex_array = vertex_arrays[id];
	CHECK_GL_ERROR(glDeleteVertexArrays(1, &vertex_array.id));
	CHECK_GL_ERROR(glDeleteBuffers(1, &vertex_array.vertex_buffer.id));
	CHECK_GL_ERROR(glDeleteBuffers(1, &vertex_array.index_buffer.id));
	vertex_array = {};
}

template<typename Vertex, typename Index>
class vertex_array {
public:

	friend class generic_vertex_array;

	vertex_array() {
		id = create_vertex_array(vertex_specification{ std::begin(Vertex::attributes), std::end(Vertex::attributes) });
	}

	vertex_array(const std::vector<Vertex>& vertices, const std::vector<Index>& indices) : vertex_array{} {
		set(vertices, indices);
	}

	vertex_array(const vertex_array&) = delete;

	vertex_array(vertex_array&& that) {
		std::swap(id, that.id);
	}

	~vertex_array() {
		if (id != -1) {
			delete_vertex_array(id);
		}
	}

	vertex_array& operator=(const vertex_array&) = delete;

	vertex_array& operator=(vertex_array&& that) {
		std::swap(id, that.id);
		return *this;
	}

	void set_vertices(const std::vector<Vertex>& vertices) {
		set_vertex_array_vertices(id, reinterpret_cast<const std::uint8_t*>(&vertices[0]), vertices.size() * sizeof(Vertex));
	}

	void set_vertices(const std::uint8_t* vertices, std::size_t vertex_count) {
		set_vertex_array_vertices(id, vertices, vertex_count * sizeof(Vertex));
	}

	void set_indices(const std::vector<Index>& indices) {
		set_vertex_array_indices(id, reinterpret_cast<const std::uint8_t*>(&indices[0]), indices.size() * sizeof(Index), sizeof(Index));
	}

	void set_indices(const std::uint8_t* indices, std::size_t index_count) {
		set_vertex_array_indices(id, indices, index_count * sizeof(Index), sizeof(Index));
	}

	void set(const std::vector<Vertex>& vertices, const std::vector<Index>& indices) {
		set_vertices(vertices);
		set_indices(indices);
	}

	void set(const std::uint8_t* vertices, std::size_t vertex_count, const std::uint8_t* indices, std::size_t index_count) {
		set_vertices(vertices, vertex_count);
		set_indices(indices, index_count);
	}

	void bind() const {
		bind_vertex_array(id);
	}

	void draw() const {
		draw_vertex_array(id);
	}

	void draw(std::size_t offset, std::size_t count) const {
		draw_vertex_array(id, offset * sizeof(Index), count);
	}

private:

	int id{ -1 };

};

}
