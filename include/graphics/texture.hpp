#pragma once

#include "vector2.hpp"

namespace nfwk {

class surface;

enum class scale_option { nearest_neighbour, linear };

class texture {
public:

	texture();
	texture(const surface& surface, scale_option scaling, bool mipmap);
	texture(const surface& surface);
	texture(const texture&) = delete;
	texture(texture&&) = delete;

	~texture();

	texture& operator=(const texture&) = delete;
	texture& operator=(texture&&) = delete;

	void bind() const;
	void bind(int slot) const;

	void load(const surface& surface, scale_option scaling, bool mipmap);
	void load(const surface& surface);
	void load_from_screen(int bottom_y, int x, int y, int width, int height);

	vector2i size() const;
	int width() const;
	int height() const;

private:

	const int id;

};

}
