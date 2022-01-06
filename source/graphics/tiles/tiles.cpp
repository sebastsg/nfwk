#include "graphics/tiles/tiles.hpp"
#include "graphics/tiles/layer.hpp"
#include "graphics/tiles/tile.hpp"
#include "graphics/tiles/chunk.hpp"
#include "graphics/tiles/bordered_renderer.hpp"
#include "graphics/tiles/autotile_renderer.hpp"
#include "graphics/ortho_camera.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"
#include "io.hpp"
#include "log.hpp"

#include <algorithm>

namespace nfwk::tiles {

// todo: tile types should also include height, so there is no need to do "hill" tile which is just grass with one y higher.

map::map(std::shared_ptr<texture> tileset_texture, int total_tile_types)
	: tileset_texture{ tileset_texture }, total_tile_types{ total_tile_types } {
}

map::~map() {

}

vector2i map::tile_index_from_mouse(const mouse& mouse, const ortho_camera& camera) {
	return active_layer->position_to_tile_index(camera.mouse_position(mouse));
}

void map::write(io_stream& stream) {

}

void map::read(io_stream& stream) {

}

std::shared_ptr<layer> map::make_layer(int depth, int chunk_tiles_per_axis, vector2i grid, renderer::method method) {
	const int active_depth{ active_layer ? active_layer->depth : depth };
	auto layer = layers.emplace_back(std::make_shared<tiles::layer>(depth, chunk_tiles_per_axis, grid, method));
	if (method == renderer::method::bordered) {
		layer->set_renderer(std::make_unique<bordered_renderer>(total_tile_types, grid.x, *tileset_texture));
	} else if (method == renderer::method::autotile) {
		layer->set_renderer(std::make_unique<autotile_renderer>(grid, total_tile_types, *tileset_texture));
	} else {
		error("tiles", "Invalid render method.");
	}
	std::sort(layers.begin(), layers.end(), [](const auto& a, const auto& b) {
		return a->depth < b->depth;
	});
	layer->on_load_area.start_forwarding_to(on_load_area);
	change_layer(active_depth);
	return layer;
}

void map::delete_layer(int depth) {
	/*std::erase_if(layers, [depth](const auto& layer) {
		return layer->depth == depth;
	});*/
}

void map::change_layer(int depth) {
	active_layer = find_layer(depth);
}

void map::show_layer(int depth) {
	if (auto layer = find_layer(depth)) {
		layer->active = true;
	}
}

void map::hide_layer(int depth) {
	if (auto layer = find_layer(depth)) {
		layer->active = false;
	}
}

std::shared_ptr<layer> map::find_layer(int depth) {
	for (auto& layer : layers) {
		if (layer->depth == depth) {
			return layer;
		}
	}
	return nullptr;
}

std::optional<tile> map::get(int x, int y) {
	return active_layer->get(x, y);
}

void map::set(int x, int y, unsigned char type) {
	active_layer->set(x, y, type);
}

void map::draw(shader& shader) {
	tileset_texture->bind();
	for (auto& layer : layers) {
		layer->draw(shader);
	}
}

void map::view(const ortho_camera& camera) {
	for (auto& layer : layers) {
		layer->view(camera);
	}
}

std::shared_ptr<texture> map::get_tileset_texture() {
	return tileset_texture;
}

}
