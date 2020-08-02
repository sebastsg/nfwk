#pragma once

#include "graphics/tiles/renderer.hpp"

#include <unordered_map>

namespace no::tiles {

class tile;

class autotile_renderer : public renderer {
public:

	autotile_renderer(vector2i size, int types, int tileset_texture);

	void render_area(layer& layer, int x, int y, int width, int height) override;
	void render_chunk(layer& layer, chunk& chunk) override;

	void load_main_tiles(int count);
	void load_group(int primary, int sub, int x, int y);

private:

	std::unordered_map<unsigned int, vector2i> uv;
	vector2i tile_size;
	vector2f tileset_size;

	std::optional<vector2i> find_uv(const tile& tile) const;
	void load_tile(unsigned char top_left, unsigned char top_right, unsigned char bottom_left, unsigned char bottom_right, int x, int y);

};


}
