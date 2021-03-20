#pragma once

#include "graphics/vertex_array.hpp"

namespace nfwk {

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

	void draw() const;
	void draw(std::size_t offset, std::size_t count) const;
	bool exists() const;

	std::size_t size_of_index() const;

private:

	int id{ -1 };
	std::size_t index_size{ 0 };

};

}
