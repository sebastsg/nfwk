#pragma once

#include "vertex.hpp"

namespace no {

int create_vertex_array(const vertex_specification& specification);
void bind_vertex_array(int id);
void set_vertex_array_vertices(int id, const uint8_t* buffer, size_t size);
void set_vertex_array_indices(int id, const uint8_t* buffer, size_t size, size_t element_size);
void draw_vertex_array(int id);
void draw_vertex_array(int id, size_t offset, int count);
void delete_vertex_array(int id);

template<typename Vertex, typename Index>
class vertex_array {
public:

	friend class generic_vertex_array;

	vertex_array() {
		id = create_vertex_array(vertex_specification{ std::begin(Vertex::attributes), std::end(Vertex::attributes) });
	}

	vertex_array(const std::vector<Vertex>& vertices, const std::vector<Index>& indices) : vertex_array{} {
		set(vertices, indices);
	}

	vertex_array(const vertex_array&) = delete;

	vertex_array(vertex_array&& that) {
		std::swap(id, that.id);
	}

	~vertex_array() {
		if (id != -1) {
			delete_vertex_array(id);
		}
	}

	vertex_array& operator=(const vertex_array&) = delete;

	vertex_array& operator=(vertex_array&& that) {
		std::swap(id, that.id);
		return *this;
	}

	void set_vertices(const std::vector<Vertex>& vertices) {
		set_vertex_array_vertices(id, reinterpret_cast<const uint8_t*>(&vertices[0]), vertices.size() * sizeof(Vertex));
	}

	void set_vertices(const uint8_t* vertices, size_t vertex_count) {
		set_vertex_array_vertices(id, vertices, vertex_count * sizeof(Vertex));
	}

	void set_indices(const std::vector<Index>& indices) {
		set_vertex_array_indices(id, reinterpret_cast<const uint8_t*>(&indices[0]), indices.size() * sizeof(Index), sizeof(Index));
	}

	void set_indices(const uint8_t* indices, size_t index_count) {
		set_vertex_array_indices(id, indices, index_count * sizeof(Index), sizeof(Index));
	}

	void set(const std::vector<Vertex>& vertices, const std::vector<Index>& indices) {
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
		draw_vertex_array(id, offset * sizeof(Index), count);
	}

private:

	int id{ -1 };

};
}
