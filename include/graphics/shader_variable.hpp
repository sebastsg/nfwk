#pragma once

#include "vector4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <vector>
#include <optional>

namespace nfwk {

class shader_variable {
public:

	std::optional<int> location;

	shader_variable(int shader_id, std::string_view name);

	void set(int value) const;
	void set(float value) const;
	void set(const vector2f& vector) const;
	void set(const vector3f& vector) const;
	void set(const vector4f& vector) const;
	void set(const glm::mat4& matrix) const;
	void set(vector2f* vector, std::size_t count) const;
	void set(const std::vector<glm::mat4>& matrices) const;
	void set(const glm::mat4* matrices, std::size_t count) const;

	template<typename Transform>
	void set(const Transform& transform) const {
		set(transform.to_matrix4());
	}

	bool exists() const;

private:

	void bind_shader() const;

	const int shader_id;

};

}
