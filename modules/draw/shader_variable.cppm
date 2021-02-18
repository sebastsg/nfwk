module;

#include <glew/glew.h>

#include "assert.hpp"
#include "gl_debug.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

export module nfwk.draw:shader_variable;

import std.core;
import nfwk.core;

export namespace nfwk {

class shader_variable {
public:

	std::optional<int> location;

	shader_variable() = default;

	shader_variable(int program_id, const std::string& name) {
		CHECK_GL_ERROR(location = glGetUniformLocation(program_id, name.c_str()));
		ASSERT(location.has_value());
	}

	void set(int value) const {
		CHECK_GL_ERROR(glUniform1i(location.value(), value));
	}

	void set(float value) const {
		CHECK_GL_ERROR(glUniform1f(location.value(), value));
	}

	void set(const vector2f& vector) const {
		CHECK_GL_ERROR(glUniform2fv(location.value(), 1, &vector.x));
	}

	void set(const vector3f& vector) const {
		CHECK_GL_ERROR(glUniform3fv(location.value(), 1, &vector.x));
	}

	void set(const vector4f& vector) const {
		CHECK_GL_ERROR(glUniform4fv(location.value(), 1, &vector.x));
	}

	void set(const glm::mat4& matrix) const {
		CHECK_GL_ERROR(glUniformMatrix4fv(location.value(), 1, false, glm::value_ptr(matrix)));
	}

	void set(vector2f* vector, std::size_t count) const {
		CHECK_GL_ERROR(glUniform2fv(location.value(), static_cast<int>(count), &vector[0].x));
	}

	void set(const std::vector<glm::mat4>& matrices) const {
		CHECK_GL_ERROR(glUniformMatrix4fv(location.value(), static_cast<int>(matrices.size()), GL_FALSE, glm::value_ptr(matrices[0])));
	}

	void set(const glm::mat4* matrices, std::size_t count) const {
		CHECK_GL_ERROR(glUniformMatrix4fv(location.value(), static_cast<int>(count), GL_FALSE, glm::value_ptr(matrices[0])));
	}

	void set(const transform2& transform) const {
		set(transform.to_matrix4());
	}

	void set(const transform3& transform) const {
		set(transform.to_matrix4());
	}

	bool exists() const {
		return location.has_value();
	}

};

}
