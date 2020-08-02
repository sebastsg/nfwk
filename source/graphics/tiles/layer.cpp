#include "graphics/tiles/layer.hpp"
#include "graphics/tiles/tile.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"
#include "graphics/ortho_camera.hpp"
#include "transform.hpp"
#include "math.hpp"
#include "debug.hpp"
#include "graphics/tiles/tiles.hpp"

#include "graphics/font.hpp"

namespace no::tiles {

layer::layer(int depth, int chunk_tiles_per_axis, vector2i grid, renderer::method method)
	: depth{ depth }, chunk_tiles_per_axis{ chunk_tiles_per_axis }, grid{ grid }, method{ method } {
	ASSERT(chunk_tiles_per_axis > 1 && chunk_tiles_per_axis < max_chunk_tiles_per_axis);
}

void layer::render() {
	if (dirty) {
		for (auto& chunk : chunks) {
			chunk.render(*renderer);
		}
		dirty = false;
	}
}

void layer::draw() {
	set_shader_model(transform2{ 0.0f, 1.0f });
	bind_texture(get_tileset_texture());
	for (auto& chunk : chunks) {
		if (chunk.rendered) {
			chunk.rendered->quads.bind();
			chunk.rendered->quads.draw();
		}
	}
}

void layer::view(const ortho_camera& camera) {
	if (!active) {
		return;
	}
	for (auto& chunk : chunks) {
		chunk.visible = false;
	}
	const auto [x1, y1] = position_to_chunk_index(camera.position());
	const auto [x2, y2] = position_to_chunk_index(camera.size()) + vector2i{ x1, y1 };
	for (int x{ x1 - 1 }; x <= x2 + 1; x++) {
		for (int y{ y1 - 1 }; y <= y2 + 1; y++) {
			if (auto chunk = get_chunk(x, y)) {
				chunk->visible = true;
				continue;
			}
			auto& chunk = create_chunk(x, y);
			const auto [tile_x, tile_y] = chunk.get_tile_index();
			events.load_area.emit(depth, tile_x, tile_y, chunk_tiles_per_axis, chunk_tiles_per_axis);
			chunk.visible = true;
		}
	}
	render();
}

vector2i layer::tile_index_to_chunk_index(int x, int y) const {
	return { divide_leftwards(x, chunk_tiles_per_axis), divide_leftwards(y, chunk_tiles_per_axis) };
}

vector2f layer::tile_index_to_position(int x, int y) const {
	return vector2i{ x * grid.x, y * grid.y }.to<float>();
}

vector2i layer::position_to_tile_index(vector2f position) const {
	const auto [x, y] = position.to<int>();
	return { divide_leftwards(x, grid.x), divide_leftwards(y, grid.y) };
}

vector2i layer::position_to_chunk_index(vector2f position) const {
	const auto [x, y] = position.to<int>();
	const auto [w, h] = chunk_tiles_per_axis * grid;
	return { divide_leftwards(x, w), divide_leftwards(y, h) };
}

chunk& layer::create_chunk(int x, int y) {
	return chunks.emplace_back(x, y, chunk_tiles_per_axis, grid, this);
}

chunk* layer::get_chunk(int x, int y) {
	for (auto& chunk : chunks) {
		if (chunk.is_at(x, y)) {
			return &chunk;
		}
	}
	return nullptr;
}

chunk* layer::find_chunk(vector2f position) {
	position.floor();
	const auto [x, y] = position_to_chunk_index(position);
	if (auto chunk = get_chunk(x, y)) {
		return chunk;
	} else {
		return nullptr;
	}
}

std::optional<tile> layer::get(int x, int y) {
	const auto [chunk_x, chunk_y] = tile_index_to_chunk_index(x, y);
	if (const auto* chunk = get_chunk(chunk_x, chunk_y)) {
		return chunk->get_tile(x, y);
	} else {
		return std::nullopt;
	}
}

chunk* layer::get_chunk_on_tile(int x, int y) {
	const auto [chunk_x, chunk_y] = tile_index_to_chunk_index(x, y);
	return get_chunk(chunk_x, chunk_y);
}

void layer::set(int x, int y, unsigned char type) {
	if (auto chunk = get_chunk_on_tile(x, y)) {
		chunk->set_tile(x, y, tile{ type });
		if (autotile_neighbours) {
			if (auto neighbour = get_chunk_on_tile(x - 1, y - 1)) {
				auto old_tile = neighbour->get_tile(x - 1, y - 1);
				old_tile.bottom_right = type;
				neighbour->set_tile(x - 1, y - 1, old_tile);
			}
			if (auto neighbour = get_chunk_on_tile(x, y - 1)) {
				auto old_tile = neighbour->get_tile(x, y - 1);
				old_tile.bottom_left = type;
				old_tile.bottom_right = type;
				neighbour->set_tile(x, y - 1, old_tile);
			}
			if (auto neighbour = get_chunk_on_tile(x + 1, y - 1)) {
				auto old_tile = neighbour->get_tile(x + 1, y - 1);
				old_tile.bottom_left = type;
				neighbour->set_tile(x + 1, y - 1, old_tile);
			}
			if (auto neighbour = get_chunk_on_tile(x - 1, y)) {
				auto old_tile = neighbour->get_tile(x - 1, y);
				old_tile.top_right = type;
				old_tile.bottom_right = type;
				neighbour->set_tile(x - 1, y, old_tile);
			}
			if (auto neighbour = get_chunk_on_tile(x + 1, y)) {
				auto old_tile = neighbour->get_tile(x + 1, y);
				old_tile.top_left = type;
				old_tile.bottom_left = type;
				neighbour->set_tile(x + 1, y, old_tile);
			}
			if (auto neighbour = get_chunk_on_tile(x - 1, y + 1)) {
				auto old_tile = neighbour->get_tile(x - 1, y + 1);
				old_tile.top_right = type;
				neighbour->set_tile(x - 1, y + 1, old_tile);
			}
			if (auto neighbour = get_chunk_on_tile(x, y + 1)) {
				auto old_tile = neighbour->get_tile(x, y + 1);
				old_tile.top_left = type;
				old_tile.top_right = type;
				neighbour->set_tile(x, y + 1, old_tile);
			}
			if (auto neighbour = get_chunk_on_tile(x + 1, y + 1)) {
				auto old_tile = neighbour->get_tile(x + 1, y + 1);
				old_tile.top_left = type;
				neighbour->set_tile(x + 1, y + 1, old_tile);
			}
		}
		dirty = true;
	}
}

void layer::set_renderer(std::unique_ptr<tiles::renderer> new_renderer) {
	renderer = std::move(new_renderer);
}

int layer::tiles_per_axis_in_chunk() const {
	return chunk_tiles_per_axis;
}

renderer& layer::get_renderer() {
	return *renderer;
}

}
