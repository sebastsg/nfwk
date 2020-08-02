#pragma once

#include "graphics/tiles/renderer.hpp"

#include <array>

namespace no::tiles {

// todo: bordered renderer still has bugs.
class bordered_renderer : public renderer {
public:

	bordered_renderer(int types, int size, int tileset_texture);

	void render_area(layer& layer, int x, int y, int width, int height) override;
	void render_chunk(layer& layer, chunk& chunk) override;

private:

	enum class location { middle, left, right, top, bottom, total };

	void render_bordered_tile(layer& layer, vector2f position, int middle, int left, int right, int top, int bottom);
	void register_subtile(int type, location location, int x, int y, int w, int h);

	std::vector<std::array<vector4f, static_cast<int>(location::total)>> tiles;
	vector2f tileset_size;
	float tile_size{ 0.0f };
	vector2f x_size;
	vector2f y_size;

};

}
