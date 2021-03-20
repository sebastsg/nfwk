#pragma once

#include <vector>

namespace nfwk {

template<typename Vertex, typename Index>
struct vertex_array_data {

	std::vector<Vertex> vertices;
	std::vector<Index> indices;

	bool operator==(const vertex_array_data& that) const {
		if (vertices.size() != that.vertices.size()) {
			return false;
		}
		if (indices.size() != that.indices.size()) {
			return false;
		}
		for (std::size_t i{ 0 }; i < vertices.size(); i++) {
			if (memcmp(&vertices[i], &that.vertices[i], sizeof(Vertex))) {
				return false;
			}
		}
		for (std::size_t i{ 0 }; i < indices.size(); i++) {
			if (indices[i] != that.indices[i]) {
				return false;
			}
		}
		return true;
	}

	bool operator!=(const vertex_array_data& that) const {
		return !operator==(that);
	}

};

}
