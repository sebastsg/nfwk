#include "platform.hpp"
#include "graphics/gl/gl.hpp"
#include "graphics/vertex_array.hpp"
#include "log.hpp"
#include "assert.hpp"

namespace nfwk::gl {

int gl_pixel_format(pixel_format format) {
	switch (format) {
	case pixel_format::rgba: return GL_RGBA;
	case pixel_format::bgra: return GL_BGRA;
	default: return GL_RGBA;
	}
}

int gl_scale_option(scale_option scaling, bool mipmap) {
	switch (scaling) {
	case scale_option::nearest_neighbor: return mipmap ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
	case scale_option::linear: return mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
	default: return GL_NEAREST;
	}
}

}

namespace nfwk {

static std::vector<gl::gl_vertex_array> vertex_arrays;
static int bound_vertex_array{ -1 };

static std::size_t size_of_attribute_component(attribute_component type) {
	switch (type) {
	case attribute_component::is_float: return sizeof(float);
	case attribute_component::is_integer: return sizeof(int);
	case attribute_component::is_byte: return sizeof(std::uint8_t);
	default: return sizeof(float);
	}
}

static void bind_vertex_array(int id) {
	bound_vertex_array = id;
	const auto& gl_vertex_array = vertex_arrays[id];
	CHECK_GL_ERROR(glBindVertexArray(gl_vertex_array.id));
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_array.vertex_buffer.id));
	CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_vertex_array.index_buffer.id));
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
	for (int i = 0; i < static_cast<int>(specification.size()); i++) {
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
	if (id != bound_vertex_array) {
		bind_vertex_array(id);
	}
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
	if (id != bound_vertex_array) {
		bind_vertex_array(id);
	}
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
		warning(draw::log, "Invalid index element size: {}", element_size);
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
	if (id != bound_vertex_array) {
		bind_vertex_array(id);
	}
	const auto& vertex_array = vertex_arrays[id];
	CHECK_GL_ERROR(glDrawElements(vertex_array.draw_mode, vertex_array.indices, vertex_array.index_type, nullptr));
}

void draw_vertex_array(int id, std::size_t offset, int count) {
	if (id != bound_vertex_array) {
		bind_vertex_array(id);
	}
	const auto& vertex_array = vertex_arrays[id];
	CHECK_GL_ERROR(glDrawElements(vertex_array.draw_mode, count, vertex_array.index_type, (void*)offset));
}

void delete_vertex_array(int id) {
	if (id < 0) {
		return;
	}
	if (id == bound_vertex_array) {
		CHECK_GL_ERROR(glBindVertexArray(0));
		CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
		CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}
	auto& vertex_array = vertex_arrays[id];
	CHECK_GL_ERROR(glDeleteVertexArrays(1, &vertex_array.id));
	CHECK_GL_ERROR(glDeleteBuffers(1, &vertex_array.vertex_buffer.id));
	CHECK_GL_ERROR(glDeleteBuffers(1, &vertex_array.index_buffer.id));
	vertex_array = {};
}

vector3i read_pixel_at(vector2i position) {
	int alignment{ 0 };
	std::uint8_t pixel[3];
	CHECK_GL_ERROR(glFlush());
	CHECK_GL_ERROR(glFinish());
	CHECK_GL_ERROR(glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment));
	CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	CHECK_GL_ERROR(glReadPixels(position.x, position.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel));
	CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_ALIGNMENT, alignment));
	return { pixel[0], pixel[1], pixel[2] };
}

}
