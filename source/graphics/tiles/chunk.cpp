#include "graphics/tiles/chunk.hpp"
#include "graphics/tiles/tile.hpp"
#include "graphics/tiles/layer.hpp"
#include "io.hpp"

namespace nfwk::tiles {

chunk::chunk(int x, int y, int tiles_per_axis, vector2i grid, tiles::layer* layer)
	: chunk_index{ x, y }, tiles_per_axis{ tiles_per_axis }, grid{ grid }, layer{ layer } {
	tile_index = chunk_index * tiles_per_axis;
	tiles.insert(tiles.begin(), tiles_per_axis * tiles_per_axis, 0);
}

void chunk::write(io_stream& stream) const {
	stream.write(chunk_index);
	stream.write(tile_index);
	stream.write(static_cast<std::int16_t>(tiles_per_axis));
	stream.write(grid);
	//stream.write_array<std::int32_t, tile>(tiles);
	stream.write<std::int8_t>(rendered ? 1 : 0);
	if (rendered) {
		stream.write(static_cast<std::int32_t>(rendered->tiles.size()));
		for (const auto& group : rendered->tiles) {
			stream.write(static_cast<std::int8_t>(group.size()));
			for (const auto& tile : group) {
				stream.write(tile.position);
				stream.write(tile.size);
				stream.write(tile.tex_coords);
			}
		}
	}
}

void chunk::read(io_stream& stream) {
	chunk_index = stream.read<vector2i>();
	tile_index = stream.read<vector2i>();
	tiles_per_axis = static_cast<int>(stream.read<std::int16_t>());
	grid = stream.read<vector2i>();
	//tiles = stream.read_array<tile, int32_t>();
	const bool has_render_data{ stream.read<std::int8_t>() != 0 };
	if (has_render_data) {
		rendered = std::make_unique<rendered_chunk>();
		const std::int32_t tile_count{ stream.read<std::int32_t>() };
		for (std::int32_t i{ 0 }; i < tile_count; i++) {
			const std::int8_t subtile_count{ stream.read<std::int8_t>() };
			auto& tile = rendered->tiles.emplace_back();
			for (std::int8_t j{ 0 }; j < subtile_count; j++) {
				const auto position = stream.read<vector2f>();
				const auto size = stream.read<vector2f>();
				const auto tex_coords = stream.read<vector4f>();
				tile.emplace_back(position, size, tex_coords);
			}
		}
	}
}

bool chunk::is_at(int x, int y) const {
	return chunk_index.x == x && chunk_index.y == y;
}

tile chunk::get_tile(int tile_x, int tile_y) const {
	return tiles[global_to_reduced_local_tile_index(tile_x, tile_y)];
}

void chunk::set_tile(int tile_x, int tile_y, tile new_tile) {
	tiles[global_to_reduced_local_tile_index(tile_x, tile_y)] = new_tile;
	dirty = true;
}

chunk* chunk::get_relative_chunk(int tile_x, int tile_y) {
	const int axis_size{ layer->tiles_per_axis_in_chunk() };
	if (tile_x >= 0 && tile_y >= 0 && tile_x < axis_size && tile_y < axis_size) {
		return this;
	}
	vector2i offset;
	if (tile_x < 0) {
		offset.x = -1;
		tile_x += axis_size;
	} else if (tile_x >= axis_size) {
		offset.x = 1;
		tile_x -= axis_size;
	}
	if (tile_y < 0) {
		offset.y = -1;
		tile_y += axis_size;
	} else if (tile_y >= axis_size) {
		offset.y = 1;
		tile_y -= axis_size;
	}
	const auto [i, j] = layer->tile_index_to_chunk_index(chunk_index.x + offset.x, chunk_index.y + offset.y);
	if (auto next = layer->get_chunk(i, j)) {
		return next->get_relative_chunk(tile_x, tile_y);
	} else {
		return nullptr;
	}
}

vector2i chunk::global_to_local_tile_index(int tile_x, int tile_y) const {
	return vector2i{ tile_x, tile_y } - get_tile_index();
}

vector2i chunk::get_tile_index() const {
	return tile_index;
}

int chunk::global_to_reduced_local_tile_index(int tile_x, int tile_y) const {
	const vector2i top_left{ get_tile_index() };
	return reduce_index(tile_x - top_left.x, tile_y - top_left.y, layer->tiles_per_axis_in_chunk());
}

void chunk::render(renderer& renderer) {
	if (visible) {
		if (dirty) {
			renderer.render_chunk(*layer, *this);
			renderer.refresh_chunk(*this);
			dirty = false;
		}
	} else if (rendered) {
		if (rendered->since_hidden.has_started()) {
			if (rendered->since_hidden.milliseconds() > layer->chunk_unload_delay_ms) {
				rendered.reset();
			}
		} else {
			rendered->since_hidden.start();
		}
	}
}

}
