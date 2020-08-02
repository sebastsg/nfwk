#pragma once

#include "vector4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <vector>
#include <string>
#include <optional>

namespace no {

class transform2;
class transform3;

class shader_variable {
public:

	std::optional<int> location;

	shader_variable() = default;
	shader_variable(int program_id, const std::string& name);

	void set(int value) const;
	void set(float value) const;
	void set(const vector2f& vector) const;
	void set(const vector3f& vector) const;
	void set(const vector4f& vector) const;
	void set(const glm::mat4& matrix) const;
	void set(vector2f* vector, size_t count) const;
	void set(const std::vector<glm::mat4>& matrices) const;
	void set(const glm::mat4* matrices, size_t count) const;

	void set(const transform2& transform) const;
	void set(const transform3& transform) const;

	bool exists() const;

};

}
