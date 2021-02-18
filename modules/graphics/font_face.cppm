module;

#include <ft2build.h>
#include FT_FREETYPE_H

#include "assert.hpp"

export module nfwk.graphics:font_face;

import std.core;
import nfwk.core;

namespace nfwk::ft {

FT_Library library{ nullptr };

}

export namespace nfwk::ft {

void initialize() {
	if (!library) {
		message("graphics", "Initializing FreeType");
		if (const auto error = FT_Init_FreeType(&library); error != FT_Err_Ok) {
			warning("graphics", "Failed to initialize FreeType. Error: {}", error);
		}
	}
}

constexpr int round(int x) {
	return (x + 32) & (-64);
}

constexpr int floor(int x) {
	return x & (-64);
}

constexpr int ceil(int x) {
	return (x + 63) & (-64);
}

constexpr int font_to_pixel(int x) {
	return x / 64;
}

}

export namespace nfwk {

class font_face {
public:

	FT_Face face;
	bool has_kerning{ false };
	bool is_scalable{ false };
	int scale_y{ 0 };
	int ascent{ 0 };
	int descent{ 0 };
	int height{ 0 };
	int line_skip{ 0 };
	int underline_offset{ 0 };
	int underline_height{ 0 };
	int glyph_overhang{ 0 };

	font_face(const std::string& path) {
		message("graphics", "Loading font {}", path);
		// note: to check how many faces a font has, face_index should be -1, then check face->num_faces
		if (const auto error = FT_New_Face(ft::library, path.c_str(), 0, &face); error != FT_Err_Ok) {
			warning("graphics", "Failed to load font: {}. Error: {}", path, error);
			return;
		}
		has_kerning = FT_HAS_KERNING(face);
		is_scalable = FT_IS_SCALABLE(face);
		if (is_scalable) {
			scale_y = face->size->metrics.y_scale;
			ascent = ft::font_to_pixel(ft::ceil(FT_MulFix(face->ascender, scale_y)));
			descent = ft::font_to_pixel(ft::ceil(FT_MulFix(face->descender, scale_y)));
			height = ascent - descent + 1;
			line_skip = ft::font_to_pixel(ft::ceil(FT_MulFix(face->height, scale_y)));
			underline_offset = ft::font_to_pixel(ft::floor(FT_MulFix(face->underline_position, scale_y)));
			underline_height = ft::font_to_pixel(ft::floor(FT_MulFix(face->underline_thickness, scale_y)));
		}
		underline_height = std::max(underline_height, 1);
		glyph_overhang = face->size->metrics.y_ppem / 10;
	}

	void set_size(int size) {
		if (FT_Set_Char_Size(face, 0, size * 64, 0, 0) != FT_Err_Ok) {
			warning("graphics", "Failed to set char size");
		}
	}

	std::uint32_t char_index(std::uint64_t character) {
		return FT_Get_Char_Index(face, static_cast<FT_ULong>(character));
	}

	bool load_glyph(int index) {
		return FT_Load_Glyph(face, index, FT_LOAD_DEFAULT) == FT_Err_Ok;
	}

	void render_glyph() {
		if (const auto error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL); error != FT_Err_Ok) {
			warning("graphics", "Failed to render glyph");
		}
	}

	void blit(std::uint32_t* destination, int left, int top, int right, int bottom, std::uint32_t color) {
		ASSERT(left >= 0);
		const int max_size{ right * bottom };
		const auto bitmap{ face->glyph->bitmap };
		const int width{ static_cast<int>(bitmap.width) };
		const int height{ static_cast<int>(bitmap.rows) };
		for (int y{ 0 }; y < height; y++) {
			for (int x{ 0 }; x < width; x++) {
				if (const int index{ y * right + x + top * right + left }; index < max_size) {
					const auto alpha = static_cast<std::uint32_t>(bitmap.buffer[y * bitmap.pitch + x]) << 24;
					destination[index] |= alpha | color;
				}
			}
		}
	}

};

}
