#pragma once

#include "graphics/vertex_array.hpp"

namespace no {

class rectangle {
public:

	rectangle(float x, float y, float width, float height);
	rectangle();

	void set_tex_coords(float x, float y, float width, float height);
	void bind() const;
	void draw() const;

private:

	vertex_array<sprite_vertex, unsigned short> vertices;

};

}
