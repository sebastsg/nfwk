#pragma once

#include <glew/glew.h>

#include "graphics/texture.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace nfwk::gl {

int gl_pixel_format(pixel_format format);
int gl_scale_option(scale_option scaling, bool mipmap);

struct gl_buffer {
	unsigned int id{ 0 };
	std::size_t allocated{ 0 };
	bool exists{ false };
};

struct gl_vertex_array {
	unsigned int id{ 0 };
	int draw_mode{ GL_TRIANGLES };
	gl_buffer vertex_buffer;
	gl_buffer index_buffer;
	std::size_t indices{ 0 };
	int index_type{ GL_UNSIGNED_SHORT };
};

struct gl_texture {
	unsigned int id{ 0 };
	vector2i size;
};

struct gl_shader {
	unsigned int id{ 0 };
	glm::mat4 model{ 1.0f };
	glm::mat4 view;
	glm::mat4 projection;
	int model_view_projection_location{ -1 };
	int model_location{ -1 };
	int view_location{ -1 };
	int projection_location{ -1 };
};

}
