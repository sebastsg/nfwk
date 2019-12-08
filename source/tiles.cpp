#include "tiles.hpp"
#include "debug.hpp"
#include "io.hpp"
#include "camera.hpp"

#include <algorithm>

namespace no::tiles {

struct {
	std::vector<std::shared_ptr<layer>> layers;
	std::shared_ptr<layer> active_layer;
	struct {
		event<int, int, int, int, int> load_area;
	} events;
	int total_tile_types{ 0 };
	int tileset_texture{ 0 };
} level;

chunk::chunk(int x, int y, int tiles_per_axis, vector2i grid, tiles::layer* layer)
	: chunk_index{ x, y }, tiles_per_axis{ tiles_per_axis }, grid{ grid }, layer{ layer } {
	tile_index = chunk_index * tiles_per_axis;
	tiles.insert(tiles.begin(), tiles_per_axis * tiles_per_axis, 0);
}

void chunk::write(io_stream& stream) const {
	stream.write(chunk_index);
	stream.write(tile_index);
	stream.write(static_cast<int16_t>(tiles_per_axis));
	stream.write(grid);
	stream.write_array<int32_t, int>(tiles);
	stream.write<int8_t>(rendered ? 1 : 0);
	if (rendered) {
		stream.write(static_cast<int32_t>(rendered->tiles.size()));
		for (const auto& group : rendered->tiles) {
			stream.write(static_cast<int8_t>(group.size()));
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
	tiles_per_axis = static_cast<int>(stream.read<int16_t>());
	grid = stream.read<vector2i>();
	tiles = stream.read_array<int, int32_t>();
	const bool has_render_data{ stream.read<int8_t>() != 0 };
	if (has_render_data) {
		rendered = std::make_unique<rendered_chunk>();
		const int32_t tile_count{ stream.read<int32_t>() };
		for (int32_t i{ 0 }; i < tile_count; i++) {
			const int8_t subtile_count{ stream.read<int8_t>() };
			auto& tile = rendered->tiles.emplace_back();
			for (int8_t j{ 0 }; j < subtile_count; j++) {
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

int chunk::get_tile(int tile_x, int tile_y) const {
	return tiles[global_to_reduced_local_tile_index(tile_x, tile_y)];
}

void chunk::set_tile(int tile_x, int tile_y, int tile) {
	tiles[global_to_reduced_local_tile_index(tile_x, tile_y)] = tile;
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
	if (auto next = layer->get_tile_chunk(i, j)) {
		return next->get_relative_chunk(tile_x, tile_y);
	} else {
		return {};
	}
}

vector2i chunk::global_to_local_tile_index(int tile_x, int tile_y) const {
	return vector2i{ tile_x, tile_y } -get_tile_index();
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
	no::set_shader_model(transform2{ 0.0f, 1.0f });
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
	for (int x{ x1 }; x <= x2; x++) {
		for (int y{ y1 }; y <= y2; y++) {
			if (auto chunk = get_tile_chunk(x, y)) {
				chunk->visible = true;
				continue;
			}
			auto& chunk = create_tile_chunk(x, y);
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

chunk& layer::create_tile_chunk(int x, int y) {
	return chunks.emplace_back(x, y, chunk_tiles_per_axis, grid, this);
}

chunk* layer::get_tile_chunk(int x, int y) {
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
	if (auto chunk = get_tile_chunk(x, y)) {
		return chunk;
	} else {
		return nullptr;
	}
}

int layer::get(int x, int y) {
	const auto [chunk_x, chunk_y] = tile_index_to_chunk_index(x, y);
	if (const auto* chunk = get_tile_chunk(chunk_x, chunk_y)) {
		return chunk->get_tile(x, y);
	} else {
		return 0;
	}
}

void layer::set(int x, int y, int tile) {
	const auto [chunk_x, chunk_y] = tile_index_to_chunk_index(x, y);
	if (auto chunk = get_tile_chunk(chunk_x, chunk_y)) {
		chunk->set_tile(x, y, tile);
		dirty = true;
	}
}

void layer::set_renderer(std::unique_ptr<tiles::renderer> new_renderer) {
	renderer = std::move(new_renderer);
}

int layer::tiles_per_axis_in_chunk() const {
	return chunk_tiles_per_axis;
}

void renderer::refresh_chunk(chunk& chunk) {
	if (!chunk.rendered) {
		return;
	}
	chunk.rendered->quads.clear();
	auto& tiles = chunk.rendered->tiles;
	std::sort(tiles.begin(), tiles.end(), [](const auto& a, const auto& b) {
		return a[0].tex_coords.x < b[0].tex_coords.x;
	});
	for (const auto& group : tiles) {
		append_tile(*chunk.rendered, group.front());
	}
	for (const auto& group : tiles) {
		for (size_t i{ 1 }; i < group.size(); i++) {
			append_tile(*chunk.rendered, group[i]);
		}
	}
	chunk.rendered->quads.refresh();
}

size_t renderer::render_tile(std::optional<size_t> parent, layer& layer, chunk& chunk, vector2f position, vector2f size, const vector4f& tex_coords) {
	if (parent.has_value()) {
		chunk.rendered->tiles[parent.value()].emplace_back(position, size, tex_coords);
		return parent.value();
	} else {
		chunk.rendered->tiles.emplace_back().emplace_back(position, size, tex_coords);
		return chunk.rendered->tiles.size() - 1;
	}
}

void renderer::clear_tile(layer& layer, vector2f position) {
	const auto [chunk_x, chunk_y] = layer.position_to_chunk_index(position);
	auto chunk = layer.get_tile_chunk(chunk_x, chunk_y);
	if (!chunk->rendered) {
		return;
	}
	auto it = std::remove_if(chunk->rendered->tiles.begin(), chunk->rendered->tiles.end(), [&](const std::vector<chunk::rendered_tile>& tile) {
		return tile[0].position.to<int>() == position.to<int>();
	});
	if (it != chunk->rendered->tiles.end()) {
		chunk->rendered->tiles.erase(it);
	}
}

void renderer::append_tile(chunk::rendered_chunk& chunk, const chunk::rendered_tile& tile) {
	const auto [x1, y1] = tile.position;
	const auto [x2, y2] = tile.position + tile.size;
	const auto [u1, v1] = tile.tex_coords.xy;
	const auto [u2, v2] = tile.tex_coords.zw;
	chunk.quads.append(
		{ { x1, y1 }, { u1, v1 } },
		{ { x2, y1 }, { u2, v1 } },
		{ { x2, y2 }, { u2, v2 } },
		{ { x1, y2 }, { u1, v2 } }
	);
}

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
			if (const int middle{ layer.get(x, y) }; middle != 0) {
				const int left{ layer.get(x - 1, y) };
				const int right{ layer.get(x + 1, y) };
				const int top{ layer.get(x, y - 1) };
				const int bottom{ layer.get(x, y + 1) };
				render_bordered_tile(layer, position, middle, left, right, top, bottom);
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
	for (int x{ start_x + 1}; x < start_x + tiles_per_axis - 1; x++) {
		for (int y{ start_y + 1 }; y < start_y + tiles_per_axis - 1; y++) {
			if (const int middle{ chunk.get_tile(x, y) }; middle != 0) {
				const int left{ chunk.get_tile(x - 1, y) };
				const int right{ chunk.get_tile(x + 1, y) };
				const int top{ chunk.get_tile(x, y - 1) };
				const int bottom{ chunk.get_tile(x, y + 1) };
				render_bordered_tile(layer, layer.tile_index_to_position(x, y), middle, left, right, top, bottom);
			}
		}
	}
}

void bordered_renderer::render_bordered_tile(layer& layer, vector2f position, int middle, int left, int right, int top, int bottom) {
	const auto [chunk_x, chunk_y] = layer.position_to_chunk_index(position);
	auto& chunk = *layer.get_tile_chunk(chunk_x, chunk_y);
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

void create(int tileset_texture, int total_tile_types) {
	level.total_tile_types = total_tile_types;
	level.tileset_texture = tileset_texture;
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

int get(int x, int y) {
	return level.active_layer->get(x, y);
}

void set(int x, int y, int tile) {
	level.active_layer->set(x, y, tile);
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
