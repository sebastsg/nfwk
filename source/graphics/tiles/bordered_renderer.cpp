#include "graphics/tiles/bordered_renderer.hpp"
#include "graphics/tiles/layer.hpp"
#include "graphics/tiles/chunk.hpp"
#include "graphics/tiles/tile.hpp"
#include "graphics/texture.hpp"

namespace no::tiles {

bordered_renderer::bordered_renderer(int types, int size, int tileset_texture) {
	tileset_size = texture_size(tileset_texture).to<float>();
	tile_size = static_cast<float>(size);
	x_size = { tile_size * 2.0f, tile_size };
	y_size = { tile_size, tile_size * 2.0f };
	const int size_1{ size };
	const int size_2{ size * 2 };
	const int size_3{ size * 3 };
	const int size_total{ size * static_cast<int>(location::total) };
	int offset_x{ 0 };
	for (int type{ 0 }; type < types; type++) {
		tiles.emplace_back();
		register_subtile(type, location::middle, size_2 + offset_x, size_2, size_1, size_1);
		register_subtile(type, location::left, offset_x, size_2, size_2, size_1);
		register_subtile(type, location::right, size_3 + offset_x, size_2, size_2, size_1);
		register_subtile(type, location::top, size_2 + offset_x, 0, size_1, size_2);
		register_subtile(type, location::bottom, size_2 + offset_x, size_3, size_1, size_2);
		offset_x += size_total;
	}
}

void bordered_renderer::render_area(layer& layer, int start_x, int start_y, int width, int height) {
	for (int x{ start_x }; x < start_x + width; x++) {
		for (int y{ start_y }; y < start_y + height; y++) {
			const vector2f position{ layer.tile_index_to_position(x, y) };
			clear_tile(layer, position);
			if (const auto middle = layer.get(x, y)) {
				const auto left = layer.get(x - 1, y)->value;
				const auto right = layer.get(x + 1, y)->value;
				const auto top = layer.get(x, y - 1)->value;
				const auto bottom = layer.get(x, y + 1)->value;
				render_bordered_tile(layer, position, middle->value, left, right, top, bottom);
			}
		}
	}
}

void bordered_renderer::render_chunk(layer& layer, chunk& chunk) {
	chunk.rendered = std::make_unique<chunk::rendered_chunk>();
	const auto [start_x, start_y] = chunk.get_tile_index();
	const int tiles_per_axis{ layer.tiles_per_axis_in_chunk() };
	render_area(layer, start_x, start_y, tiles_per_axis, 1);
	render_area(layer, start_x, start_y, 1, tiles_per_axis);
	render_area(layer, start_x + tiles_per_axis - 1, start_y, 1, tiles_per_axis);
	render_area(layer, start_x, start_y + tiles_per_axis - 1, tiles_per_axis, 1);
	for (int x{ start_x + 1 }; x < start_x + tiles_per_axis - 1; x++) {
		for (int y{ start_y + 1 }; y < start_y + tiles_per_axis - 1; y++) {
			if (const auto middle = chunk.get_tile(x, y); middle.value != 0) {
				const auto left = chunk.get_tile(x - 1, y).value;
				const auto right = chunk.get_tile(x + 1, y).value;
				const auto top = chunk.get_tile(x, y - 1).value;
				const auto bottom = chunk.get_tile(x, y + 1).value;
				render_bordered_tile(layer, layer.tile_index_to_position(x, y), middle.value, left, right, top, bottom);
			}
		}
	}
}

void bordered_renderer::render_bordered_tile(layer& layer, vector2f position, int middle, int left, int right, int top, int bottom) {
	const auto [chunk_x, chunk_y] = layer.position_to_chunk_index(position);
	auto& chunk = *layer.get_chunk(chunk_x, chunk_y);
	size_t parent{ render_tile({}, layer, chunk, position, tile_size, tiles[middle][static_cast<int>(location::middle)]) };
	if (left != middle) {
		const vector2f left_position{ position.x - tile_size, position.y };
		render_tile(parent, layer, chunk, left_position, x_size, tiles[middle][static_cast<int>(location::left)]);
	}
	if (right != middle) {
		const vector2f right_position{ position.x, position.y };
		render_tile(parent, layer, chunk, right_position, x_size, tiles[middle][static_cast<int>(location::right)]);
	}
	if (top != middle) {
		const vector2f top_position{ position.x, position.y - tile_size };
		render_tile(parent, layer, chunk, top_position, y_size, tiles[middle][static_cast<int>(location::top)]);
	}
	if (bottom != middle) {
		const vector2f bottom_position{ position.x, position.y };
		render_tile(parent, layer, chunk, bottom_position, y_size, tiles[middle][static_cast<int>(location::bottom)]);
	}
}

void bordered_renderer::register_subtile(int type, location location, int x, int y, int w, int h) {
	tiles[type][static_cast<int>(location)] = {
		static_cast<float>(x) / tileset_size.x,
		static_cast<float>(y) / tileset_size.y,
		static_cast<float>(x + w) / tileset_size.x,
		static_cast<float>(y + h) / tileset_size.y
	};
}

}
