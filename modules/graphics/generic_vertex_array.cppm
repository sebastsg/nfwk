export module nfwk.graphics:generic_vertex_array;

import std.core;

export namespace nfwk {

class generic_vertex_array {
public:

	generic_vertex_array() = default;
	generic_vertex_array(const generic_vertex_array&) = delete;

	generic_vertex_array(generic_vertex_array&& that) noexcept {
		std::swap(id, that.id);
		std::swap(index_size, that.index_size);
	}

	//template<typename Vertex, typename Index>
	//generic_vertex_array(vertex_array<Vertex, Index>&& that) : index_size{ sizeof(Index) } {
	//	std::swap(id, that.id);
	//}

	~generic_vertex_array() {
		//delete_vertex_array(id);
	}

	generic_vertex_array& operator=(const generic_vertex_array&) = delete;

	generic_vertex_array& operator=(generic_vertex_array&& that) noexcept {
		std::swap(id, that.id);
		std::swap(index_size, that.index_size);
		return *this;
	}

	void bind() const {
		//bind_vertex_array(id);
	}

	void draw() const {
		//draw_vertex_array(id);
	}

	void draw(size_t offset, std::size_t count) const {
		//draw_vertex_array(id, offset, count);
	}

	bool exists() const {
		return id != -1;
	}

	std::size_t size_of_index() const {
		return index_size;
	}

private:

	int id{ -1 };
	std::size_t index_size{ 0 };

};

}
