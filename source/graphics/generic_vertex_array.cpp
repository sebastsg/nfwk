#include "graphics/generic_vertex_array.hpp"

namespace no {

generic_vertex_array::generic_vertex_array(generic_vertex_array&& that) noexcept {
	std::swap(id, that.id);
	std::swap(index_size, that.index_size);
}

generic_vertex_array::~generic_vertex_array() {
	delete_vertex_array(id);
}

generic_vertex_array& generic_vertex_array::operator=(generic_vertex_array&& that) noexcept {
	std::swap(id, that.id);
	std::swap(index_size, that.index_size);
	return *this;
}

void generic_vertex_array::bind() const {
	bind_vertex_array(id);
}

void generic_vertex_array::draw() const {
	draw_vertex_array(id);
}

void generic_vertex_array::draw(size_t offset, size_t count) const {
	draw_vertex_array(id, offset, count);
}

bool generic_vertex_array::exists() const {
	return id != -1;
}

size_t generic_vertex_array::size_of_index() const {
	return index_size;
}

}
