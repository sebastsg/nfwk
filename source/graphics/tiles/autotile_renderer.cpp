#include "graphics/tiles/autotile_renderer.hpp"
#include "graphics/tiles/layer.hpp"
#include "graphics/tiles/tile.hpp"
#include "graphics/texture.hpp"

namespace no::tiles {

autotile_renderer::autotile_renderer(vector2i size, int types, int tileset_texture) : tile_size{ size } {
	tileset_size = texture_size(tileset_texture).to<float>();
	load_main_tiles(types);
}

void autotile_renderer::render_area(layer& layer, int start_x, int start_y, int width, int height) {
	for (int x{ start_x }; x < start_x + width; x++) {
		for (int y{ start_y }; y < start_y + height; y++) {
			const vector2f position{ layer.tile_index_to_position(x, y) };
			clear_tile(layer, position);
			if (const auto current = layer.get(x, y)) {
				vector4i current_uv_i{ uv[current->value].x, uv[current->value].y, tile_size.x, tile_size.y };
				current_uv_i.z += current_uv_i.x;
				current_uv_i.w += current_uv_i.y;
				const vector4f current_uv{ current_uv_i.to<float>() / tileset_size.x };
				const auto [chunk_x, chunk_y] = layer.position_to_chunk_index(position);
				auto& chunk = *layer.get_chunk(chunk_x, chunk_y);
				render_tile(std::nullopt, layer, chunk, position, tile_size.to<float>(), current_uv);
			}
		}
	}
}

void autotile_renderer::render_chunk(layer& layer, chunk& chunk) {
	chunk.rendered = std::make_unique<chunk::rendered_chunk>();
	const auto [start_x, start_y] = chunk.get_tile_index();
	const int tiles_per_axis{ layer.tiles_per_axis_in_chunk() };
	render_area(layer, start_x, start_y, tiles_per_axis, 1);
	render_area(layer, start_x, start_y, 1, tiles_per_axis);
	render_area(layer, start_x + tiles_per_axis - 1, start_y, 1, tiles_per_axis);
	render_area(layer, start_x, start_y + tiles_per_axis - 1, tiles_per_axis, 1);
	for (int x{ start_x + 1 }; x < start_x + tiles_per_axis - 1; x++) {
		for (int y{ start_y + 1 }; y < start_y + tiles_per_axis - 1; y++) {
			if (const auto current = chunk.get_tile(x, y); current.value != 0) {
				vector4i current_uv_i{ uv[current.value].x, uv[current.value].y, tile_size.x, tile_size.y };
				current_uv_i.z += current_uv_i.x;
				current_uv_i.w += current_uv_i.y;
				const vector4f current_uv{ current_uv_i.to<float>() / tileset_size.x };
				render_tile(std::nullopt, layer, chunk, layer.tile_index_to_position(x, y), tile_size.to<float>(), current_uv);
			}
		}
	}
}

std::optional<vector2i> autotile_renderer::find_uv(const tile& tile) const {
	if (const auto it = uv.find(tile.value); it != uv.end()) {
		return it->second;
	} else {
		return std::nullopt;
	}
}

void autotile_renderer::load_main_tiles(int count) {
	for (unsigned char i{ 1 }; i <= count; i++) {
		load_tile(i, i, i, i, i - 1, 0);
	}
}

void autotile_renderer::load_tile(unsigned char top_left, unsigned char top_right, unsigned char bottom_left, unsigned char bottom_right, int x, int y) {
	uv[tile{ top_left, top_right, bottom_left, bottom_right }.value] = { x * tile_size.x, y * tile_size.y };
}

void autotile_renderer::load_group(int primary, int sub, int x, int y) {
	load_tile(primary, primary, primary, sub, x, y);
	load_tile(primary, primary, sub, sub, x + 1, y);
	load_tile(primary, primary, sub, primary, x + 2, y);
	load_tile(primary, sub, primary, primary, x, y + 1);
	load_tile(sub, sub, primary, primary, x + 1, y + 1);
	load_tile(sub, primary, primary, primary, x + 2, y + 1);
	y += 2;
	load_tile(sub, sub, sub, primary, x, y);
	//load_tile(sub, sub, primary, primary, x + 1, y);
	load_tile(sub, sub, primary, sub, x + 2, y);
	load_tile(sub, primary, sub, sub, x, y + 1);
	//load_tile(primary, primary, sub, sub, x + 1, y + 1);
	load_tile(primary, sub, sub, sub, x + 2, y + 1);
	y += 2;
	load_tile(sub, primary, primary, sub, x, y);
	load_tile(primary, sub, primary, sub, x + 2, y);
	load_tile(primary, sub, sub, primary, x, y + 1);
	load_tile(sub, primary, sub, primary, x + 2, y + 1);
}

}
