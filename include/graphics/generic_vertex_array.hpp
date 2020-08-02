#pragma once

#include "graphics/vertex_array.hpp"

namespace no {

class generic_vertex_array {
public:

	generic_vertex_array() = default;
	generic_vertex_array(const generic_vertex_array&) = delete;
	generic_vertex_array(generic_vertex_array&&) noexcept;

	template<typename Vertex, typename Index>
	generic_vertex_array(vertex_array<Vertex, Index>&& that) : index_size{ sizeof(Index) } {
		std::swap(id, that.id);
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

}
