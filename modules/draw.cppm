module;

#include <glew/glew.h>
#include "gl_debug.hpp"

export module nfwk.draw;

import std.core;
import nfwk.core;
export import :shader;
export import :shader_variable;
export import :texture;
export import :vertex_array;
export import :window;

export namespace nfwk {

vector3i read_pixel_at(vector2i position) {
	int alignment{ 0 };
	std::uint8_t pixel[3]{};
	CHECK_GL_ERROR(glFlush());
	CHECK_GL_ERROR(glFinish());
	CHECK_GL_ERROR(glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment));
	CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	CHECK_GL_ERROR(glReadPixels(position.x, position.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel));
	CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_ALIGNMENT, alignment));
	return { pixel[0], pixel[1], pixel[2] };
}

template<typename Shape, typename Transform>
void draw_shape(const Shape& shape, const Transform& transform) {
	set_shader_model(transform);
	shape.bind();
	shape.draw();
}

}

#if 0
export std::ostream& operator<<(std::ostream& out, nfwk::swap_interval interval) {
	switch (interval) {
	case nfwk::swap_interval::late: return out << "Late";
	case nfwk::swap_interval::immediate: return out << "Immediate";
	case nfwk::swap_interval::sync: return out << "Sync";
	default: return out << "Unknown";
	}
}
#endif
