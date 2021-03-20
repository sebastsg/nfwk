#include "platform.hpp"
#include "graphics/gl/gl.hpp"
#include "graphics/texture.hpp"
#include "log.hpp"

#include <array>

namespace nfwk {

static std::vector<gl::gl_texture> gl_textures;
static std::array<int, 32> gl_bound_textures;
static int gl_active_texture_slot{ 0 };

int create_texture() {
	std::optional<int> id;
	for (std::size_t i{ 0 }; i < gl_textures.size(); i++) {
		if (gl_textures[i].id == 0) {
			id = static_cast<int>(i);
			break;
		}
	}
	if (!id.has_value()) {
		gl_textures.emplace_back();
		id = static_cast<int>(gl_textures.size()) - 1;
	}
	CHECK_GL_ERROR(glGenTextures(1, &gl_textures[id.value()].id));
	return id.value();
}

texture::texture() : id{ create_texture() } {}

texture::texture(const surface& surface, scale_option scaling, bool mipmaps) : id{ create_texture() } {
	load(surface, scaling, mipmaps);
}

texture::texture(const surface& surface) : id{ create_texture() } {
	load(surface);
}

texture::~texture() {
	CHECK_GL_ERROR(glDeleteTextures(1, &gl_textures[id].id));
	gl_textures[id] = {};
}

void texture::bind() const {
	gl_bound_textures[gl_active_texture_slot] = gl_textures[id].id;
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, gl_textures[id].id));
}

void texture::bind(int slot) const {
	CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0 + slot));
	gl_active_texture_slot = slot;
	gl_bound_textures[gl_active_texture_slot] = gl_textures[id].id;
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, gl_textures[id].id));
}

void texture::load(const surface& surface, scale_option scaling, bool mipmap) {
	auto& gl_texture = gl_textures[id];
	gl_texture.size = surface.dimensions();
	const auto format = gl::gl_pixel_format(surface.format());
	const auto bound_texture_id = gl_bound_textures[gl_active_texture_slot];
	bind();
	CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gl_texture.size.x, gl_texture.size.y, 0, format, GL_UNSIGNED_BYTE, surface.data()));
	if (mipmap) {
		CHECK_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
	}
	CHECK_GL_ERROR(glTextureParameteri(gl_texture.id, GL_TEXTURE_MAG_FILTER, gl::gl_scale_option(scaling, false)));
	CHECK_GL_ERROR(glTextureParameteri(gl_texture.id, GL_TEXTURE_MIN_FILTER, gl::gl_scale_option(scaling, mipmap)));
	CHECK_GL_ERROR(glTextureParameteri(gl_texture.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	CHECK_GL_ERROR(glTextureParameteri(gl_texture.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, bound_texture_id));
}

void texture::load(const surface& surface) {
	load(surface, scale_option::nearest_neighbour, false);
}

void texture::load_from_screen(int bottom_y, int x, int y, int width, int height) {
	gl_textures[id].size = { width, height };
	const auto bound_texture_id = gl_bound_textures[gl_active_texture_slot];
	bind();
	CHECK_GL_ERROR(glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, bottom_y - y - height, width, height, 0));
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, bound_texture_id));
}

vector2i texture::size() const {
	return gl_textures[id].size;
}

int texture::width() const {
	return gl_textures[id].size.x;
}

int texture::height() const {
	return gl_textures[id].size.y;
}

}
