export module nfwk.tiles:renderer;

import std.core;
import nfwk.core;
import :chunk;
import :tile;
import :layer;

export namespace nfwk::tiles {

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

	void refresh_chunk(chunk& chunk) {
		if (!chunk.rendered) {
			return;
		}
		chunk.rendered->quads.clear();
		auto& tiles = chunk.rendered->tiles;
		std::sort(tiles.begin(), tiles.end(), [](const auto& a, const auto& b) {
			return a[0].tex_coords.x < b[0].tex_coords.x;
		});
		for (const auto& group : tiles) {
			append_tile(*chunk.rendered, group.front());
		}
		for (const auto& group : tiles) {
			for (std::size_t i{ 1 }; i < group.size(); i++) {
				append_tile(*chunk.rendered, group[i]);
			}
		}
		chunk.rendered->quads.refresh();
	}

protected:

	std::size_t render_tile(std::optional<size_t> parent, layer& layer, chunk& chunk, vector2f position, vector2f size, const vector4f& tex_coords) {
		if (parent.has_value()) {
			chunk.rendered->tiles[parent.value()].emplace_back(position, size, tex_coords);
			return parent.value();
		} else {
			chunk.rendered->tiles.emplace_back().emplace_back(position, size, tex_coords);
			return chunk.rendered->tiles.size() - 1;
		}
	}

	void clear_tile(layer& layer, vector2f position) {
		const auto [chunk_x, chunk_y] = layer.position_to_chunk_index(position);
		auto chunk = layer.get_chunk(chunk_x, chunk_y);
		if (!chunk->rendered) {
			return;
		}
		auto it = std::remove_if(chunk->rendered->tiles.begin(), chunk->rendered->tiles.end(), [&](const std::vector<chunk::rendered_tile>& tile) {
			return tile[0].position.to<int>() == position.to<int>();
		});
		if (it != chunk->rendered->tiles.end()) {
			chunk->rendered->tiles.erase(it);
		}
	}

	void append_tile(chunk::rendered_chunk& chunk, const chunk::rendered_tile& tile) {
		const auto [x1, y1] = tile.position;
		const auto [x2, y2] = tile.position + tile.size;
		const auto [u1, v1] = tile.tex_coords.xy;
		const auto [u2, v2] = tile.tex_coords.zw;
		chunk.quads.append(
			{ { x1, y1 }, { u1, v1 } },
			{ { x2, y1 }, { u2, v1 } },
			{ { x2, y2 }, { u2, v2 } },
			{ { x1, y2 }, { u1, v2 } }
		);
	}

};

}
