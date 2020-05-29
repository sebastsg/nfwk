#pragma once

#include "platform.hpp"

#if ENABLE_GRAPHICS

#include "transform.hpp"
#include "containers.hpp"
#include "timer.hpp"
#include "vertex.hpp"

namespace no {

#define MEASURE_REDUNDANT_BIND_CALLS DEBUG_ENABLED

class surface;
class ortho_camera;
class perspective_camera;
class shader_variable;

enum class swap_interval { late, immediate, sync };
enum class scale_option { nearest_neighbour, linear };
enum class polygon_render_mode { fill, wireframe };

int create_vertex_array(const vertex_specification& specification);
void bind_vertex_array(int id);
void set_vertex_array_vertices(int id, const uint8_t* buffer, size_t size);
void set_vertex_array_indices(int id, const uint8_t* buffer, size_t size, size_t element_size);
void draw_vertex_array(int id);
void draw_vertex_array(int id, size_t offset, int count);
void delete_vertex_array(int id);

int create_texture();
int create_texture(const surface& surface, scale_option scaling, bool mipmap);
int create_texture(const surface& surface);
void bind_texture(int id);
void bind_texture(int id, int slot);
void load_texture(int id, const surface& surface, scale_option scaling, bool mipmap);
void load_texture(int id, const surface& surface);
void load_texture_from_screen(int id, int bottom_y, int x, int y, int width, int height);
vector2i texture_size(int id);
void delete_texture(int id);

int create_shader_from_source(std::string_view vertex, std::string_view fragment);
int create_shader(const std::string& path);
void bind_shader(int id);
shader_variable get_shader_variable(const std::string& name);
void set_shader_model(const glm::mat4& transform);
void set_shader_model(const transform2& transform);
void set_shader_model(const transform3& transform);
void set_shader_view_projection(const glm::mat4& view, const glm::mat4& projection);
void set_shader_view_projection(const ortho_camera& camera);
void set_shader_view_projection(const perspective_camera& camera);
void delete_shader(int id);

void set_polygon_render_mode(polygon_render_mode mode);
vector3i read_pixel_at(vector2i position);

long long total_redundant_bind_calls();

class shader_variable {
public:

	int location{ -1 };

	shader_variable() = default;
	shader_variable(int program_id, const std::string& name);

	// debatable whether or not these should be const
	// it's useful for const draw functions
	void set(int value) const;
	void set(float value) const;
	void set(const vector2f& vector) const;
	void set(const vector3f& vector) const;
	void set(const vector4f& vector) const;
	void set(const glm::mat4& matrix) const;
	void set(const transform2& transform) const;
	void set(const transform3& transform) const;
	void set(vector2f* vector, size_t count) const;
	void set(const std::vector<glm::mat4>& matrices) const;
	void set(const glm::mat4* matrices, size_t count) const;

	bool exists() const;

};

template<typename V, typename I>
class vertex_array {
public:

	friend class generic_vertex_array;

	vertex_array() {
		id = create_vertex_array(vertex_specification(std::begin(V::attributes), std::end(V::attributes)));
	}

	vertex_array(const std::vector<V>& vertices, const std::vector<I>& indices) : vertex_array{} {
		set(vertices, indices);
	}

	vertex_array(const vertex_array&) = delete;

	vertex_array(vertex_array&& that) {
		std::swap(id, that.id);
	}

	~vertex_array() {
		delete_vertex_array(id);
	}

	vertex_array& operator=(const vertex_array&) = delete;

	vertex_array& operator=(vertex_array&& that) {
		std::swap(id, that.id);
		return *this;
	}

	void set_vertices(const std::vector<V>& vertices) {
		set_vertex_array_vertices(id, (uint8_t*)&vertices[0], vertices.size() * sizeof(V));
	}

	void set_vertices(const uint8_t* vertices, size_t vertex_count) {
		set_vertex_array_vertices(id, vertices, vertex_count * sizeof(V));
	}

	void set_indices(const std::vector<I>& indices) {
		set_vertex_array_indices(id, (uint8_t*)&indices[0], indices.size() * sizeof(I), sizeof(I));
	}

	void set_indices(const uint8_t* indices, size_t index_count) {
		set_vertex_array_indices(id, indices, index_count * sizeof(I), sizeof(I));
	}

	void set(const std::vector<V>& vertices, const std::vector<I>& indices) {
		set_vertices(vertices);
		set_indices(indices);
	}

	void set(const uint8_t* vertices, size_t vertex_count, const uint8_t* indices, size_t index_count) {
		set_vertices(vertices, vertex_count);
		set_indices(indices, index_count);
	}

	void bind() const {
		bind_vertex_array(id);
	}

	void draw() const {
		draw_vertex_array(id);
	}

	void draw(size_t offset, size_t count) const {
		draw_vertex_array(id, offset * sizeof(I), count);
	}

private:

	int id{ -1 };

};

class generic_vertex_array {
public:

	generic_vertex_array() = default;
	generic_vertex_array(const generic_vertex_array&) = delete;
	generic_vertex_array(generic_vertex_array&&) noexcept;

	template<typename V, typename I>
	generic_vertex_array(vertex_array<V, I>&& that) {
		std::swap(id, that.id);
		index_size = sizeof(I);
	}

	~generic_vertex_array();

	generic_vertex_array& operator=(const generic_vertex_array&) = delete;
	generic_vertex_array& operator=(generic_vertex_array&&) noexcept;

	void bind() const;
	void draw() const;
	void draw(size_t offset, size_t count) const;
	bool exists() const;

	size_t size_of_index() const;

private:

	int id{ -1 };
	size_t index_size{ 0 };

};

class model {
public:

	friend class model_instance;
	friend class skeletal_animator;

	model() = default;
	model(const model&) = delete;
	model(model&&) noexcept;

	~model() = default;

	model& operator=(const model&) = delete;
	model& operator=(model&&) noexcept;

	int index_of_animation(const std::string& name) const;
	int total_animations() const;
	model_animation& animation(int index);
	const model_animation& animation(int index) const;
	model_node& node(int index);
	int total_nodes() const;
	glm::mat4 bone(int index) const;

	template<typename V, typename I>
	void load(const model_data<V, I>& model) {
		if (model.shape.vertices.empty()) {
			WARNING("Failed to load model");
			return;
		}
		mesh = { std::move(vertex_array<V, I>{model.shape.vertices, model.shape.indices }) };
		root_transform = model.transform;
		min_vertex = model.min;
		max_vertex = model.max;
		nodes = model.nodes;
		bones = model.bones;
		animations = model.animations;
		texture = model.texture;
		model_name = model.name;
		size_t vertices = model.shape.vertices.size();
		size_t indices = model.shape.indices.size();
		drawable = (vertices > 0 && indices > 0);
	}

	template<typename V, typename I>
	void load(const std::string& path) {
		model_data<V, I> model;
		import_model(path, model);
		if (model.shape.vertices.empty()) {
			WARNING("Failed to load model: " << path);
			return;
		}
		load(model);
	}
	
	void bind() const;
	void draw() const;

	bool is_drawable() const;

	vector3f min() const;
	vector3f max() const;
	vector3f size() const;

	std::string texture_name() const;
	std::string name() const;

private:

	generic_vertex_array mesh;
	glm::mat4 root_transform;
	std::vector<glm::mat4> bones;
	std::vector<model_node> nodes;
	std::vector<model_animation> animations;
	vector3f min_vertex;
	vector3f max_vertex;
	bool drawable{ false };
	std::string texture;
	std::string model_name;

};

class rectangle {
public:

	rectangle(float x, float y, float width, float height);
	rectangle();

	void set_tex_coords(float x, float y, float width, float height);
	void bind() const;
	void draw() const;

private:

	vertex_array<sprite_vertex, unsigned short> vertices;

};

class sprite_animation {
public:

	int frames{ 1 };
	float fps{ 10.0f };

	void update(float delta);
	void draw(vector2f position, vector2f size) const;
	void draw(vector2f position, int texture) const;
	void draw(const transform2& transform) const;

	void pause();
	void resume();
	bool is_paused() const;
	void set_frame(int frame);
	void set_tex_coords(vector2f position, vector2f size);
	void set_tex_coords(vector4f tex_coords);
	void start_looping();
	void stop_looping();
	bool is_looping() const;
	bool is_done() const;

private:

	rectangle rectangle;
	int current_frame{ 0 };
	float sub_frame{ 0.0f };
	int previous_frame{ 0 };
	bool paused{ false };
	vector2f uv_position;
	vector2f uv_size{ 1.0f };
	bool looping{ true };
	bool done{ false };

};

template<typename V>
class quad {
public:

	quad() {
		set({}, {}, {}, {});
	}
	
	quad(const V& top_left, const V& top_right, const V& bottom_left, const V& bottom_right) {
		set(top_left, top_right, bottom_right, bottom_left);
	}

	void set(const V& top_left, const V& top_right, const V& bottom_left, const V& bottom_right) {
		vertices.set({ top_left, top_right, bottom_right, bottom_left }, { 0, 1, 2, 3, 2, 0 });
	}

	void bind() const {
		vertices.bind();
	}

	void draw() const {
		vertices.draw();
	}

private:

	vertex_array<V, unsigned short> vertices;

};

template<typename V, typename I>
class quad_array {
public:

	quad_array() = default;

	quad_array(const quad_array&) = delete;

	quad_array(quad_array&& that) noexcept : shape{ std::move(that.shape) } {
		std::swap(vertices, that.vertices);
		std::swap(indices, that.indices);
	}

	quad_array& operator=(const quad_array&) = delete;

	quad_array& operator=(quad_array&& that) noexcept {
		std::swap(shape, that.shape);
		std::swap(vertices, that.vertices);
		std::swap(indices, that.indices);
		return *this;
	}

	void append(const V& v1, const V& v2, const V& v3, const V& v4) {
		const I i{ static_cast<I>(vertices.size()) };
		vertices.insert(vertices.end(), { v1, v2, v3, v4 });
		indices.insert(indices.end(), { (I)i, (I)(i + 1), (I)(i + 2), (I)i, (I)(i + 3), (I)(i + 2) });
	}

	void clear() {
		vertices.clear();
		indices.clear();
	}

	void refresh() {
		if (!vertices.empty()) {
			shape.set(vertices, indices);
		}
	}

	void bind() const {
		shape.bind();
	}

	void draw() const {
		shape.draw();
	}

private:

	vertex_array<V, I> shape;
	std::vector<V> vertices;
	std::vector<I> indices;

};

template<typename S, typename M>
void draw_shape(const S& shape, const M& transform) {
	set_shader_model(transform);
	shape.bind();
	shape.draw();
}

}

std::ostream& operator<<(std::ostream& out, no::swap_interval interval);

#endif
