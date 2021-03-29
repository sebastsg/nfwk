#include "platform.hpp"
#include "graphics/gl/gl.hpp"
#include "graphics/shader.hpp"
#include "graphics/ortho_camera.hpp"
#include "graphics/perspective_camera.hpp"
#include "log.hpp"
#include "assert.hpp"

#include "glm/gtc/type_ptr.hpp"

namespace nfwk {

static std::vector<gl::gl_shader> shaders;
static int bound_shader{ -1 };

static int create_shader_script(std::u8string_view source, unsigned int type) {
	CHECK_GL_ERROR(const unsigned int id{ glCreateShader(type) });
	const char* source_data = reinterpret_cast<const char*>(source.data());
	const auto source_size = static_cast<int>(source.size());
	CHECK_GL_ERROR(glShaderSource(id, 1, &source_data, &source_size));
	CHECK_GL_ERROR(glCompileShader(id));
	int length{ 0 };
	char buffer[1024];
	CHECK_GL_ERROR(glGetShaderInfoLog(id, 1024, &length, buffer));
	if (length > 0) {
		info(draw::log, u8"Shader info log:\n{}", buffer);
	}
	return static_cast<int>(id);
}

static std::vector<std::u8string> find_vertex_shader_attributes(const std::u8string_view source) {
	std::vector<std::u8string> attributes;
	std::size_t index{ source.find(u8"in ") };
	while (index != std::u8string::npos) {
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
		if (end_index == std::u8string::npos) {
			break; // shader syntax error, so no need to log this
		}
		attributes.emplace_back(source.substr(index, end_index - index));
		index = source.find(u8"in ", end_index);
	}
	return attributes;
}

void shader::load_from_source(std::u8string_view vertex_source, std::u8string_view fragment_source) {
	id = -1;
	for (int i{ 0 }; i < static_cast<int>(shaders.size()); i++) {
		if (shaders[i].id == 0) {
			id = i;
			break;
		}
	}
	if (id == -1) {
		shaders.emplace_back();
		id = static_cast<int>(shaders.size()) - 1;
	}
	auto& gl_shader = shaders[id];
	CHECK_GL_ERROR(gl_shader.id = glCreateProgram());

	auto attributes = find_vertex_shader_attributes(vertex_source.data());
	info(draw::log, u8"Attributes: {}", attributes);
	int vertex_shader_id = create_shader_script(vertex_source.data(), GL_VERTEX_SHADER);
	CHECK_GL_ERROR(glAttachShader(gl_shader.id, vertex_shader_id));
	for (int location{ 0 }; location < static_cast<int>(attributes.size()); location++) {
		const auto* attribute_string = reinterpret_cast<const char*>(attributes[location].c_str());
		CHECK_GL_ERROR(glBindAttribLocation(gl_shader.id, location, attribute_string));
	}
	const int fragment_shader_id{ create_shader_script(fragment_source.data(), GL_FRAGMENT_SHADER) };
	CHECK_GL_ERROR(glAttachShader(gl_shader.id, fragment_shader_id));
	CHECK_GL_ERROR(glLinkProgram(gl_shader.id));
	CHECK_GL_ERROR(glDeleteShader(vertex_shader_id));
	CHECK_GL_ERROR(glDeleteShader(fragment_shader_id));

	char buffer[1024];
	int length{ 0 };
	CHECK_GL_ERROR(glGetProgramInfoLog(gl_shader.id, 1024, &length, buffer));
	if (length > 0) {
		info(draw::log, u8"Shader program log {}:\n{}", gl_shader.id, buffer);
	}
	CHECK_GL_ERROR(glValidateProgram(gl_shader.id));
	int status{ 0 };
	CHECK_GL_ERROR(glGetProgramiv(gl_shader.id, GL_VALIDATE_STATUS, &status));
	if (status == GL_FALSE) {
		error(draw::log, u8"Failed to validate shader program with id {}", gl_shader.id);
	}
	ASSERT(status != GL_FALSE);

	bind();
	CHECK_GL_ERROR(gl_shader.model_view_projection_location = glGetUniformLocation(gl_shader.id, "model_view_projection"));
	CHECK_GL_ERROR(gl_shader.model_location = glGetUniformLocation(gl_shader.id, "model"));
	CHECK_GL_ERROR(gl_shader.view_location = glGetUniformLocation(gl_shader.id, "view"));
	CHECK_GL_ERROR(gl_shader.projection_location = glGetUniformLocation(gl_shader.id, "projection"));
}

shader::shader(const std::filesystem::path& path) {
	message(draw::log, u8"Loading shader {}", path);
	load_from_source(read_file(path / u8"vertex.glsl"), read_file(path / u8"fragment.glsl"));
}

shader::shader(std::u8string_view vertex, std::u8string_view fragment) {
	message(draw::log, u8"Loading shader from source");
	load_from_source(vertex, fragment);
}

shader::~shader() {
	if (bound_shader == id) {
		CHECK_GL_ERROR(glUseProgram(0));
	}
	CHECK_GL_ERROR(glDeleteProgram(shaders[id].id));
	shaders[id] = {};
}

void shader::bind() const {
	bound_shader = id;
	CHECK_GL_ERROR(glUseProgram(shaders[id].id));
}

shader_variable shader::get_variable(std::u8string_view name) const {
	return { id, name };
}

void shader::set_model(const glm::mat4& transform) {
	if (bound_shader != id) {
		bind();
	}
	auto& gl_shader = shaders[id];
	gl_shader.model = transform;
	const auto model_view_projection{ gl_shader.projection * gl_shader.view * gl_shader.model };
	CHECK_GL_ERROR(glUniformMatrix4fv(gl_shader.model_view_projection_location, 1, false, glm::value_ptr(model_view_projection)));
	CHECK_GL_ERROR(glUniformMatrix4fv(gl_shader.model_location, 1, false, glm::value_ptr(gl_shader.model)));
}

void shader::set_view_projection(const glm::mat4& view, const glm::mat4& projection) {
	if (bound_shader != id) {
		bind();
	}
	auto& gl_shader = shaders[id];
	gl_shader.view = view;
	gl_shader.projection = projection;
	const auto model_view_projection = gl_shader.projection * gl_shader.view * gl_shader.model;
	CHECK_GL_ERROR(glUniformMatrix4fv(gl_shader.model_view_projection_location, 1, false, glm::value_ptr(model_view_projection)));
	CHECK_GL_ERROR(glUniformMatrix4fv(gl_shader.view_location, 1, false, glm::value_ptr(gl_shader.view)));
	CHECK_GL_ERROR(glUniformMatrix4fv(gl_shader.projection_location, 1, false, glm::value_ptr(gl_shader.projection)));
}

void set_polygon_render_mode(polygon_render_mode mode) {
	CHECK_GL_ERROR(glPolygonMode(GL_FRONT_AND_BACK, mode == polygon_render_mode::fill ? GL_FILL : GL_LINE));
}

shader_variable::shader_variable(int shader_id, std::u8string_view name) : shader_id{ shader_id } {
	CHECK_GL_ERROR(location = glGetUniformLocation(shaders[shader_id].id, reinterpret_cast<const char*>(name.data())));
	ASSERT(location != -1);
}

void shader_variable::set(int value) const {
	if (bound_shader != shader_id) {
		bind_shader();
	}
	CHECK_GL_ERROR(glUniform1i(location.value(), value));
}

void shader_variable::set(float value) const {
	if (bound_shader != shader_id) {
		bind_shader();
	}
	CHECK_GL_ERROR(glUniform1f(location.value(), value));
}

void shader_variable::set(const vector2f& vector) const {
	if (bound_shader != shader_id) {
		bind_shader();
	}
	CHECK_GL_ERROR(glUniform2fv(location.value(), 1, &vector.x));
}

void shader_variable::set(const vector3f& vector) const {
	if (bound_shader != shader_id) {
		bind_shader();
	}
	CHECK_GL_ERROR(glUniform3fv(location.value(), 1, &vector.x));
}

void shader_variable::set(const vector4f& vector) const {
	if (bound_shader != shader_id) {
		bind_shader();
	}
	CHECK_GL_ERROR(glUniform4fv(location.value(), 1, &vector.x));
}

void shader_variable::set(const glm::mat4& matrix) const {
	if (bound_shader != shader_id) {
		bind_shader();
	}
	CHECK_GL_ERROR(glUniformMatrix4fv(location.value(), 1, false, glm::value_ptr(matrix)));
}

void shader_variable::set(vector2f* vector, std::size_t count) const {
	if (bound_shader != shader_id) {
		bind_shader();
	}
	CHECK_GL_ERROR(glUniform2fv(location.value(), static_cast<int>(count), &vector[0].x));
}

void shader_variable::set(const std::vector<glm::mat4>& matrices) const {
	if (bound_shader != shader_id) {
		bind_shader();
	}
	CHECK_GL_ERROR(glUniformMatrix4fv(location.value(), static_cast<int>(matrices.size()), GL_FALSE, glm::value_ptr(matrices[0])));
}

void shader_variable::set(const glm::mat4* matrices, std::size_t count) const {
	if (bound_shader != shader_id) {
		bind_shader();
	}
	CHECK_GL_ERROR(glUniformMatrix4fv(location.value(), static_cast<int>(count), GL_FALSE, glm::value_ptr(matrices[0])));
}

bool shader_variable::exists() const {
	return location.has_value() && location != -1;
}

void shader_variable::bind_shader() const {
	bound_shader = shader_id;
	CHECK_GL_ERROR(glUseProgram(shaders[shader_id].id));
}

}
