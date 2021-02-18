module;

#include <glew/glew.h>
#include "gl_debug.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

export module nfwk.draw:shader;

import std.core;
import std.filesystem;
import nfwk.core;
import nfwk.graphics;
import :gl_structs;
import :shader_variable;

namespace nfwk {

static std::vector<gl::gl_shader> shaders;
static int bound_shader{ -1 };

static int create_shader_script(const char* source, unsigned int type) {
	CHECK_GL_ERROR(const unsigned int id{ glCreateShader(type) });
	CHECK_GL_ERROR(glShaderSource(id, 1, &source, 0));
	CHECK_GL_ERROR(glCompileShader(id));
#if ENABLE_DEBUG_LOG
	int length{ 0 };
	char buffer[1024];
	CHECK_GL_ERROR(glGetShaderInfoLog(id, 1024, &length, buffer));
	if (length > 0) {
		info("graphics", "Shader info log:\n{}", buffer);
	}
#endif
	return static_cast<int>(id);
}

std::ostream& operator<<(std::ostream& out, const std::vector<std::string>& strings) {
	for (size_t i{ 0 }; i < strings.size(); i++) {
		out << strings[i];
		if (strings.size() - 1 > i) {
			out << ", ";
		}
	}
	return out;
}

std::vector<std::string> find_vertex_shader_attributes(const std::string_view source) {
	std::vector<std::string> attributes;
	std::size_t index{ source.find("in ") };
	while (index != std::string::npos) {
		index += 3;
		// skip data type
		while (source.size() > index) {
			index++;
			if (source[index] == ' ') {
				index++;
				break;
			}
		}
		// read name
		const std::size_t end_index{ source.find(';', index) };
		if (end_index == std::string::npos) {
			break; // shader syntax error, so no need to log this
		}
		attributes.emplace_back(source.substr(index, end_index - index));
		index = source.find("in ", end_index);
	}
	return attributes;
}

}

export namespace nfwk {

class ortho_camera;
class perspective_camera;

enum class polygon_render_mode { fill, wireframe };

void bind_shader(int id) {
	if (bound_shader != id) {
		bound_shader = id;
		CHECK_GL_ERROR(glUseProgram(shaders[id].id));
	}
}

int create_shader_from_source(std::string_view vertex_source, std::string_view fragment_source) {
	std::optional<int> id;
	for (size_t i{ 0 }; i < shaders.size(); i++) {
		if (shaders[i].id == 0) {
			id = static_cast<int>(i);
			break;
		}
	}
	if (!id.has_value()) {
		shaders.emplace_back();
		id = static_cast<int>(shaders.size()) - 1;
	}
	auto& shader = shaders[id.value()];
	CHECK_GL_ERROR(shader.id = glCreateProgram());

	const auto& attributes = find_vertex_shader_attributes(vertex_source.data());
	message("graphics", "Attributes: {}", attributes);
	int vertex_shader_id = create_shader_script(vertex_source.data(), GL_VERTEX_SHADER);
	CHECK_GL_ERROR(glAttachShader(shader.id, vertex_shader_id));
	for (int location{ 0 }; location < (int)attributes.size(); location++) {
		CHECK_GL_ERROR(glBindAttribLocation(shader.id, location, attributes[location].c_str()));
	}
	const int fragment_shader_id{ create_shader_script(fragment_source.data(), GL_FRAGMENT_SHADER) };
	CHECK_GL_ERROR(glAttachShader(shader.id, fragment_shader_id));
	CHECK_GL_ERROR(glLinkProgram(shader.id));
	CHECK_GL_ERROR(glDeleteShader(vertex_shader_id));
	CHECK_GL_ERROR(glDeleteShader(fragment_shader_id));

#if ENABLE_DEBUG_LOG
	char buffer[1024];
	int length{ 0 };
	CHECK_GL_ERROR(glGetProgramInfoLog(shader.id, 1024, &length, buffer));
	if (length > 0) {
		info("graphics", "Shader program log {}:\n{}" << shader.id, buffer);
	}
	CHECK_GL_ERROR(glValidateProgram(shader.id));
	int status{ 0 };
	CHECK_GL_ERROR(glGetProgramiv(shader.id, GL_VALIDATE_STATUS, &status));
	if (status == GL_FALSE) {
		error("graphics", "Failed to validate shader program with id {}", shader.id);
	}
	ASSERT(status != GL_FALSE);
#endif

	bind_shader(id.value());
	CHECK_GL_ERROR(shader.model_view_projection_location = glGetUniformLocation(shader.id, "model_view_projection"));
	CHECK_GL_ERROR(shader.model_location = glGetUniformLocation(shader.id, "model"));
	CHECK_GL_ERROR(shader.view_location = glGetUniformLocation(shader.id, "view"));
	CHECK_GL_ERROR(shader.projection_location = glGetUniformLocation(shader.id, "projection"));
	return id.value();
}

int create_shader(const std::filesystem::path& path) {
	message("graphics", "Loading shader {}", path);
	return 0; // return create_shader_from_source(read_file(path / "vertex.glsl"), read_file(path / "fragment.glsl"));
}

void delete_shader(int id) {
	CHECK_GL_ERROR(glDeleteProgram(shaders[id].id));
	shaders[id] = {};
}

shader_variable get_shader_variable(const std::string& name) {
	return { static_cast<int>(shaders[bound_shader].id), name };
}

void set_shader_model(const glm::mat4& transform) {
	auto& shader = shaders[bound_shader];
	shader.model = transform;
	const auto model_view_projection{ shader.projection * shader.view * shader.model };
	CHECK_GL_ERROR(glUniformMatrix4fv(shader.model_view_projection_location, 1, false, glm::value_ptr(model_view_projection)));
	CHECK_GL_ERROR(glUniformMatrix4fv(shader.model_location, 1, false, glm::value_ptr(shader.model)));
}

void set_shader_model(const transform2& transform) {
	set_shader_model(transform.to_matrix4());
}

void set_shader_model(const transform3& transform) {
	set_shader_model(transform.to_matrix4());
}

void set_shader_view_projection(const glm::mat4& view, const glm::mat4& projection) {
	auto& shader = shaders[bound_shader];
	shader.view = view;
	shader.projection = projection;
	const auto model_view_projection = shader.projection * shader.view * shader.model;
	CHECK_GL_ERROR(glUniformMatrix4fv(shader.model_view_projection_location, 1, false, glm::value_ptr(model_view_projection)));
	CHECK_GL_ERROR(glUniformMatrix4fv(shader.view_location, 1, false, glm::value_ptr(shader.view)));
	CHECK_GL_ERROR(glUniformMatrix4fv(shader.projection_location, 1, false, glm::value_ptr(shader.projection)));
}

void set_shader_view_projection(const ortho_camera& camera) {
	set_shader_view_projection(camera.view(), camera.projection());
}

void set_shader_view_projection(const perspective_camera& camera) {
	set_shader_view_projection(camera.view(), camera.projection());
}

void set_polygon_render_mode(polygon_render_mode mode) {
	CHECK_GL_ERROR(glPolygonMode(GL_FRONT_AND_BACK, mode == polygon_render_mode::fill ? GL_FILL : GL_LINE));
}

}
