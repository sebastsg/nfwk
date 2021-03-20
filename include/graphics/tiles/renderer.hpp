#pragma once

#include "chunk.hpp"

#include <optional>

namespace nfwk::tiles {

class layer;

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

	std::size_t render_tile(std::optional<std::size_t> parent, layer& layer, chunk& chunk, vector2f position, vector2f size, const vector4f& tex_coords);
	void clear_tile(layer& layer, vector2f position);
	void append_tile(chunk::rendered_chunk& chunk, const chunk::rendered_tile& tile);

};

}
