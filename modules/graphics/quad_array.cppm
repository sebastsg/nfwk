export module nfwk.graphics:quad_array;

import std.core;

export namespace nfwk {

template<typename Vertex, typename Index>
class quad_array {
public:

	quad_array() = default;

	quad_array(const quad_array&) = delete;

	quad_array(quad_array&& that) noexcept {//: shape{ std::move(that.shape) } {
		std::swap(vertices, that.vertices);
		std::swap(indices, that.indices);
	}

	quad_array& operator=(const quad_array&) = delete;

	quad_array& operator=(quad_array&& that) noexcept {
		//std::swap(shape, that.shape);
		std::swap(vertices, that.vertices);
		std::swap(indices, that.indices);
		return *this;
	}

	void append(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4) {
		const auto i = static_cast<Index>(vertices.size());
		vertices.insert(vertices.end(), { v1, v2, v3, v4 });
		indices.insert(indices.end(), {
			i, static_cast<Index>(i + 1), static_cast<Index>(i + 2),
			i, static_cast<Index>(i + 3), static_cast<Index>(i + 2)
		});
	}

	void clear() {
		vertices.clear();
		indices.clear();
	}

	void refresh() {
		if (!vertices.empty()) {
			//shape.set(vertices, indices);
		}
	}

	void bind() const {
		//shape.bind();
	}

	void draw() const {
		//shape.draw();
	}

private:

	//vertex_array<Vertex, Index> shape;
	std::vector<Vertex> vertices;
	std::vector<Index> indices;

};

}
