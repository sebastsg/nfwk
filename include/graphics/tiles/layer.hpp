#pragma once

#include "graphics/tiles/renderer.hpp"
#include "event.hpp"
#include "graphics/vertex.hpp"

namespace nfwk {

class ortho_camera;
class shader;

}

namespace nfwk::tiles {

class tile;

class layer {
public:

	event<int, int, int, int, int> on_load_area;

	bool autotile_neighbours{ true };
	int depth{ 0 };
	bool dirty{ false };
	bool active{ true };
	int chunk_unload_delay_ms{ 5000 };

	layer(int depth, int chunk_tiles_per_axis, vector2i grid, renderer::method method);
	layer(const layer&) = delete;
	layer(layer&&) = delete;

	layer& operator=(const layer&) = delete;
	layer& operator=(layer&&) = delete;

	void render();
	void draw(shader& shader);
	void view(const ortho_camera& camera);

	vector2i tile_index_to_chunk_index(int x, int y) const;
	vector2f tile_index_to_position(int x, int y) const;
	vector2i position_to_tile_index(vector2f position) const;
	vector2i position_to_chunk_index(vector2f position) const;

	chunk& create_chunk(int x, int y);
	chunk* get_chunk(int x, int y);
	chunk* get_chunk_on_tile(int x, int y);
	chunk* find_chunk(vector2f position);

	std::optional<tile> get(int x, int y);
	void set(int x, int y, unsigned char tile);

	void set_renderer(std::unique_ptr<renderer> renderer);
	int tiles_per_axis_in_chunk() const;

	const std::vector<chunk>& get_chunks() const {
		return chunks;
	}

	template<typename Renderer>
	Renderer& get_renderer() {
		return static_cast<Renderer&>(*renderer);
	}

private:

	std::vector<chunk> chunks;
	int chunk_tiles_per_axis{ 32 };
	vector2i grid{ 16 };
	renderer::method method{ renderer::method::custom };
	std::unique_ptr<renderer> renderer;

};

}
