#pragma once

#include "graphics/tiles/renderer.hpp"
#include "event.hpp"

namespace nfwk {
class ortho_camera;
class io_stream;
class mouse;
class shader;
class texture;
}

namespace nfwk::tiles {

class tile;

class map {
public:

	event<int, int, int, int, int> on_load_area;

	map(std::shared_ptr<texture> tileset_texture, int total_tile_types);
	map(const map&) = delete;
	map(map&&) = delete;

	~map();

	map& operator=(const map&) = delete;
	map& operator=(map&&) = delete;

	vector2i tile_index_from_mouse(const mouse& mouse, const ortho_camera& camera);

	void write(io_stream& stream);
	void read(io_stream& stream);

	std::shared_ptr<layer> make_layer(int depth, int chunk_tiles_per_axis, vector2i grid, renderer::method method);
	void delete_layer(int depth);
	void change_layer(int depth);
	void show_layer(int depth);
	void hide_layer(int depth);

	std::shared_ptr<layer> find_layer(int depth);

	std::optional<tile> get(int x, int y);
	void set(int x, int y, unsigned char tile);

	void draw(shader& shader);
	void view(const ortho_camera& camera);

	std::shared_ptr<texture> get_tileset_texture();

private:

	std::vector<std::shared_ptr<layer>> layers;
	std::shared_ptr<layer> active_layer;
	int total_tile_types{ 0 };
	std::shared_ptr<texture> tileset_texture;

};

}
