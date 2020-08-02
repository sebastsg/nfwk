#pragma once

#include "vector3.hpp"

namespace no::debug {
long long get_redundant_texture_bind_calls();
}

namespace no {

enum class swap_interval { late, immediate, sync };

vector3i read_pixel_at(vector2i position);

template<typename Shape, typename Transform>
void draw_shape(const Shape& shape, const Transform& transform) {
	set_shader_model(transform);
	shape.bind();
	shape.draw();
}

}

std::ostream& operator<<(std::ostream& out, no::swap_interval interval);
