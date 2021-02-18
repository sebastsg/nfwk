module;

#include <glew/glew.h>
#include "gl_debug.hpp"

export module nfwk.draw:texture;

import std.core;
import nfwk.core;
import nfwk.graphics;
import :gl_structs;

namespace nfwk {
std::vector<gl::gl_texture> textures;
}

export namespace nfwk {

#if 0

static long long redundant_texture_bind_calls{ 0 };

void measure_redundant_bind_call(int id) {
	int active_texture_id{ 0 };
	CHECK_GL_ERROR(glGetIntegerv(GL_TEXTURE_BINDING_2D, &active_texture_id));
	if (active_texture_id == textures[id].id) {
		redundant_texture_bind_calls++;
	}
}

long long get_redundant_texture_bind_calls() {
	return redundant_texture_bind_calls;
}

#endif

enum class scale_option { nearest_neighbour, linear };

}

export namespace nfwk::gl {

int gl_scale_option(scale_option scaling, bool mipmap) {
	switch (scaling) {
	case scale_option::nearest_neighbour: return mipmap ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
	case scale_option::linear: return mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
	default: return GL_NEAREST;
	}
}

}

export namespace nfwk {

int create_texture() {
	std::optional<int> id;
	for (size_t i{ 0 }; i < textures.size(); i++) {
		if (textures[i].id == 0) {
			id = static_cast<int>(i);
			break;
		}
	}
	if (!id.has_value()) {
		textures.emplace_back();
		id = static_cast<int>(textures.size()) - 1;
	}
	auto& texture = textures[id.value()];
	CHECK_GL_ERROR(glGenTextures(1, &texture.id));
	return id.value();
}

void bind_texture(int id) {
#if 0
	measure_redundant_bind_call(id);
#endif
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, textures[id].id));
}

void bind_texture(int id, int slot) {
	CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0 + slot));
#if 0
	measure_redundant_bind_call(id);
#endif
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, textures[id].id));
}

vector2i texture_size(int id) {
	return textures[id].size;
}

void load_texture(int id, const surface& surface, scale_option scaling, bool mipmap) {
	auto& texture = textures[id];
	texture.size = surface.dimensions();
	const int format{ gl::gl_pixel_format(surface.format()) };
	bind_texture(id);
	CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.size.x, texture.size.y, 0, format, GL_UNSIGNED_BYTE, surface.data()));
	if (mipmap) {
		CHECK_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
	}
	CHECK_GL_ERROR(glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, gl::gl_scale_option(scaling, false)));
	CHECK_GL_ERROR(glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, gl::gl_scale_option(scaling, mipmap)));
	CHECK_GL_ERROR(glTextureParameteri(texture.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	CHECK_GL_ERROR(glTextureParameteri(texture.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
}

void load_texture(int id, const surface& surface) {
	load_texture(id, surface, scale_option::nearest_neighbour, false);
}

void load_texture_from_screen(int id, int bottom_y, int x, int y, int width, int height) {
	auto& texture = textures[id];
	texture.size = { width, height };
	bind_texture(id);
	CHECK_GL_ERROR(glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, bottom_y - y - height, width, height, 0));
}

int create_texture(const surface& surface, scale_option scaling, bool mipmaps) {
	const int id{ create_texture() };
	load_texture(id, surface, scaling, mipmaps);
	return id;
}

int create_texture(const surface& surface) {
	const int id{ create_texture() };
	load_texture(id, surface);
	return id;
}

void delete_texture(int id) {
	auto& texture = textures[id];
	CHECK_GL_ERROR(glDeleteTextures(1, &texture.id));
	texture = {};
}

}
