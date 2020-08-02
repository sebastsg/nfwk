#pragma once

#include "graphics/tiles/renderer.hpp"
#include "event.hpp"

namespace no {
class ortho_camera;
class io_stream;
class mouse;
}

namespace no::tiles {

class tile;

vector2i tile_index_from_mouse(const mouse& mouse, const ortho_camera& camera);

void create(int tileset_texture, int total_tile_types);
void destroy();

void write(io_stream& stream);
void read(io_stream& stream);

void make_layer(int depth, int chunk_tiles_per_axis, vector2i grid, renderer::method method);
void delete_layer(int depth);
void change_layer(int depth);
void show_layer(int depth);
void hide_layer(int depth);

std::shared_ptr<layer> find_layer(int depth);

std::optional<tile> get(int x, int y);
void set(int x, int y, unsigned char tile);

void draw();
void view(const ortho_camera& camera);

int get_tileset_texture();

[[nodiscard]] event_listener on_load_area(const std::function<void(int d, int x, int y, int w, int h)>& load);

}
