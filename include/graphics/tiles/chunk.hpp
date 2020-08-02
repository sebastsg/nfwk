#pragma once

#include "graphics/tiles/tile.hpp"
#include "graphics/quad_array.hpp"
#include "vector2.hpp"
#include "timer.hpp"

namespace no {
class io_stream;
}

namespace no::tiles {

class layer;
class renderer;

// to not overflow in index buffer. max = sqrt(65535 / 4)
static constexpr int max_chunk_tiles_per_axis{ 96 };

struct tile_vertex {
	static constexpr vertex_attribute_specification attributes[]{ 2, 2 };
	vector2f position;
	vector2f tex_coords;
};

class chunk {
public:

	struct rendered_tile {
		vector2f position;
		vector2f size;
		vector4f tex_coords;
		rendered_tile(vector2f position, vector2f size, const vector4f& tex_coords)
			: position{ position }, size{ size }, tex_coords{ tex_coords } {
		}
	};

	struct rendered_chunk {
		quad_array<tile_vertex, unsigned short> quads;
		std::vector<std::vector<rendered_tile>> tiles;
		timer since_hidden;
	};

	bool visible{ false };
	bool dirty{ false };
	std::unique_ptr<rendered_chunk> rendered;

	chunk(int x, int y, int tiles_per_axis, vector2i grid, layer* layer);
	chunk(const chunk&) = delete;
	chunk(chunk&&) = default;

	chunk& operator=(const chunk&) = delete;
	chunk& operator=(chunk&&) = default;

	void write(io_stream& stream) const;
	void read(io_stream& stream);

	bool is_at(int x, int y) const;
	tile get_tile(int x, int y) const;
	void set_tile(int x, int y, tile new_tile);
	chunk* get_relative_chunk(int x, int y);
	vector2i global_to_local_tile_index(int x, int y) const;
	vector2i get_tile_index() const;

	void render(renderer& renderer);

private:

	int global_to_reduced_local_tile_index(int x, int y) const;

	std::vector<tile> tiles;
	layer* layer{ nullptr };
	vector2i chunk_index;
	vector2i tile_index;
	int tiles_per_axis{ 0 };
	vector2i grid;

};

}
