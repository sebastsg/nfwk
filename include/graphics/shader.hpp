#pragma once

#include "graphics/shader_variable.hpp"

#include <filesystem>

namespace nfwk {

enum class polygon_render_mode { fill, wireframe };

void set_polygon_render_mode(polygon_render_mode mode);

class shader {
public:

	shader(const std::filesystem::path& path);
	shader(std::u8string_view vertex, std::u8string_view fragment);
	shader(const shader&) = delete;
	shader(shader&&) = delete;

	~shader();

	shader& operator=(const shader&) = delete;
	shader& operator=(shader&&) = delete;

	shader_variable get_variable(std::u8string_view name) const;

	void set_model(const glm::mat4& transform);
	void set_view_projection(const glm::mat4& view, const glm::mat4& projection);

	template<typename Transform>
	void set_model(const Transform& transform) {
		set_model(transform.to_matrix4());
	}

	template<typename Camera>
	void set_view_projection(const Camera& camera) {
		set_view_projection(camera.view(), camera.projection());
	}

	template<typename Shape, typename Transform>
	void draw(const Shape& shape, const Transform& transform) {
		set_model(transform);
		shape.draw();
	}

private:

	void bind() const;
	void load_from_source(std::u8string_view vertex_source, std::u8string_view fragment_source);

	int id{ -1 };

};

}
