#pragma once

#include "vector2.hpp"
#include "graphics/rectangle.hpp"

#include <memory>

namespace nfwk {

class shader;
class texture;
class ortho_camera;

class grid {
public:

	grid(std::shared_ptr<texture> texture = nullptr);
	~grid();

	void draw(shader& shader, const ortho_camera& camera, vector2f size);

private:

	rectangle shape;
	std::shared_ptr<texture> grid_texture;

};

}
