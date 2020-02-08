#pragma once

#include "event.hpp"
#include "math.hpp"
#include "draw.hpp"

#include <array>
#include <optional>
#include <unordered_map>

namespace no {
class ortho_camera;
class io_stream;
class mouse;
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

class tile {
public:

	union {
		int value{};
		unsigned char corner[4];
		struct {
			unsigned char top_left;
			unsigned char top_right;
			unsigned char bottom_left;
			unsigned char bottom_right;
		};
	};
	
	tile(unsigned char top_left, unsigned char top_right, unsigned char bottom_left, unsigned char bottom_right);
	tile(unsigned char type);
	tile() = default;

	bool is_only(unsigned char type) const;

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
	int get_tile(int x, int y) const;
	void set_tile(int x, int y, int tile);
	chunk* get_relative_chunk(int x, int y);
	vector2i global_to_local_tile_index(int x, int y) const;
	vector2i get_tile_index() const;

	void render(renderer& renderer);

private:

	int global_to_reduced_local_tile_index(int x, int y) const;

	std::vector<int> tiles;
	layer* layer{ nullptr };
	vector2i chunk_index;
	vector2i tile_index;
	int tiles_per_axis{ 0 };
	vector2i grid;

};

class renderer {
public:

	enum class method {
		custom,
		bordered, // corners as subtiles outside main tile
		autotile, // standard autotiling
		tall_autotile, // 2x wall autotile (needs to be ported from Caligo project)
	};

	virtual void render_area(layer& layer, int x, int y, int width, int height) = 0;
	virtual void render_chunk(layer& layer, chunk& chunk) = 0;

	void refresh_chunk(chunk& chunk);

protected:

	size_t render_tile(std::optional<size_t> parent, layer& layer, chunk& chunk, vector2f position, vector2f size, const vector4f& tex_coords);
	void clear_tile(layer& layer, vector2f position);
	void append_tile(chunk::rendered_chunk& chunk, const chunk::rendered_tile& tile);

};

class layer {
public:

	struct {
		event<int, int, int, int, int> load_area;
	} events;

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
	void draw();
	void view(const ortho_camera& camera);

	vector2i tile_index_to_chunk_index(int x, int y) const;
	vector2f tile_index_to_position(int x, int y) const;
	vector2i position_to_tile_index(vector2f position) const;
	vector2i position_to_chunk_index(vector2f position) const;

	chunk& create_tile_chunk(int x, int y);
	chunk* get_tile_chunk(int x, int y);
	chunk* find_chunk(vector2f position);

	int get(int x, int y);
	void set(int x, int y, int tile);

	void set_renderer(std::unique_ptr<renderer> renderer);
	int tiles_per_axis_in_chunk() const;

private:

	std::vector<chunk> chunks;
	int chunk_tiles_per_axis{ 32 };
	vector2i grid{ 16 };
	renderer::method method{ renderer::method::custom };
	std::unique_ptr<renderer> renderer;

};

// todo: bordered renderer still has bugs.
class bordered_renderer : public renderer {
public:

	bordered_renderer(int types, int size, int tileset_texture);

	void render_area(layer& layer, int x, int y, int width, int height) override;
	void render_chunk(layer& layer, chunk& chunk) override;

private:

	enum class location { middle, left, right, top, bottom, total };

	void render_bordered_tile(layer& layer, vector2f position, int middle, int left, int right, int top, int bottom);
	void register_subtile(int type, location location, int x, int y, int w, int h);

	std::vector<std::array<vector4f, static_cast<int>(location::total)>> tiles;
	vector2f tileset_size;
	float tile_size{ 0.0f };
	vector2f x_size;
	vector2f y_size;

};

class autotile_renderer : public renderer {
public:

	autotile_renderer(vector2i size, int tileset_texture);

	void render_area(layer& layer, int x, int y, int width, int height) override;
	void render_chunk(layer& layer, chunk& chunk) override;

	void load_main_tiles(int count);
	void load_group(int primary, int sub, int x, int y);

private:

	std::unordered_map<unsigned int, vector2i> uv;
	vector2i tile_size;

	vector2i get_uv(const tile& tile) const;
	void load_tile(unsigned char top_left, unsigned char top_right, unsigned char bottom_left, unsigned char bottom_right, int x, int y);

};

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

int get(int x, int y);
void set(int x, int y, int tile);

void draw();
void view(const ortho_camera& camera);

[[nodiscard]] event_listener on_load_area(const std::function<void(int d, int x, int y, int w, int h)>& load);

}
