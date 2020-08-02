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
#include "debug.hpp"

#include <algorithm>

namespace no::tiles {

// todo: tile types should also include height, so there is no need to do "hill" tile which is just grass with one y higher.

struct {
	std::vector<std::shared_ptr<layer>> layers;
	std::shared_ptr<layer> active_layer;
	struct {
		event<int, int, int, int, int> load_area;
	} events;
	int total_tile_types{ 0 };
	int tileset_texture{ 0 };
} level;

int get_tileset_texture() {
	return level.tileset_texture;
}

vector2i tile_index_from_mouse(const mouse& mouse, const ortho_camera& camera) {
	return level.active_layer->position_to_tile_index(camera.mouse_position(mouse));
}

void create(int tileset_texture, int total_tile_types) {
	level.tileset_texture = tileset_texture;
	level.total_tile_types = total_tile_types;
}

void destroy() {
	level = {};
}

void write(io_stream& stream) {

}

void read(io_stream& stream) {

}

void make_layer(int depth, int chunk_tiles_per_axis, vector2i grid, renderer::method method) {
	const int active_depth{ level.active_layer ? level.active_layer->depth : depth };
	auto layer = level.layers.emplace_back(std::make_shared<tiles::layer>(depth, chunk_tiles_per_axis, grid, method));
	if (method == renderer::method::bordered) {
		layer->set_renderer(std::make_unique<bordered_renderer>(level.total_tile_types, grid.x, level.tileset_texture));
	} else if (method == renderer::method::autotile) {
		layer->set_renderer(std::make_unique<autotile_renderer>(grid, level.total_tile_types, level.tileset_texture));
	} else {
		CRITICAL("Invalid render method.");
	}
	std::sort(level.layers.begin(), level.layers.end(), [](const auto& a, const auto& b) {
		return a->depth < b->depth;
	});
	layer->events.load_area.start_forwarding_to(level.events.load_area);
	change_layer(active_depth);
}

void delete_layer(int depth) {
	auto it = std::remove_if(level.layers.begin(), level.layers.end(), [depth](const auto& layer) {
		return layer->depth == depth;
	});
	if (it != level.layers.end()) {
		level.layers.erase(it);
	}
}

void change_layer(int depth) {
	level.active_layer = find_layer(depth);
}

void show_layer(int depth) {
	if (auto layer = find_layer(depth)) {
		layer->active = true;
	}
}

void hide_layer(int depth) {
	if (auto layer = find_layer(depth)) {
		layer->active = false;
	}
}

std::shared_ptr<layer> find_layer(int depth) {
	for (auto& layer : level.layers) {
		if (layer->depth == depth) {
			return layer;
		}
	}
	return nullptr;
}

std::optional<tile> get(int x, int y) {
	return level.active_layer->get(x, y);
}

void set(int x, int y, unsigned char type) {
	level.active_layer->set(x, y, type);
}

void draw() {
	for (auto& layer : level.layers) {
		layer->draw();
	}
}

void view(const ortho_camera& camera) {
	for (auto& layer : level.layers) {
		layer->view(camera);
	}
}

event_listener on_load_area(const std::function<void(int, int, int, int, int)>& load) {
	return level.events.load_area.listen(load);
}

}
