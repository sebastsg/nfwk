#include "font.hpp"
#include "debug.hpp"
#include "platform.hpp"
#include "unicode.hpp"

#include <filesystem>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace no {

namespace ft {

static FT_Library library{ nullptr };

static void initialize() {
	if (!library) {
		MESSAGE("Initializing FreeType");
		if (const auto error{ FT_Init_FreeType(&library) }; error != FT_Err_Ok) {
			WARNING("[Error " << error << "] Failed to initialize FreeType");
		}
	}
}

static long round(long x) {
	return (x + 32) & (-64);
}

static long floor(long x) {
	return x & (-64);
}

static long ceil(long x) {
	return (x + 63) & (-64);
}

static long font_to_pixel(long x) {
	return x / 64;
}

}

class font::font_face {
public:

	FT_Face face;
	bool has_kerning{ false };
	bool is_scalable{ false };
	long scale_y{ 0 };
	long ascent{ 0 };
	long descent{ 0 };
	long height{ 0 };
	long line_skip{ 0 };
	long underline_offset{ 0 };
	long underline_height{ 0 };
	long glyph_overhang{ 0 };

	font_face(const std::string& path) {
		MESSAGE("Loading font " << path);
		// note: to check how many faces a font has, face_index should be -1, then check face->num_faces
		if (const auto error = FT_New_Face(ft::library, path.c_str(), 0, &face); error != FT_Err_Ok) {
			WARNING("[Error " << error << "] Failed to load font: " << path);
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
		underline_height = std::max(underline_height, 1L);
		glyph_overhang = face->size->metrics.y_ppem / 10;
	}

	void set_size(int size) {
		if (FT_Set_Char_Size(face, 0, size * 64, 0, 0) != FT_Err_Ok) {
			WARNING("Failed to set char size");
		}
	}

	uint32_t char_index(uint64_t character) {
		return FT_Get_Char_Index(face, (FT_ULong)character);
	}

	bool load_glyph(int index) {
		return FT_Load_Glyph(face, index, FT_LOAD_DEFAULT) == FT_Err_Ok;
	}

	void render_glyph() {
		FT_Error error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		if (error != FT_Err_Ok) {
			WARNING("Failed to render glyph");
		}
	}

	void blit(uint32_t* destination, int left, int top, int right, int bottom, uint32_t color) {
		ASSERT(left >= 0);
		const int max_size{ right * bottom };
		const auto bitmap{ face->glyph->bitmap };
		const int width{ static_cast<int>(bitmap.width) };
		const int height{ static_cast<int>(bitmap.rows) };
		for (int y{ 0 }; y < height; y++) {
			for (int x{ 0 }; x < width; x++) {
				if (const int index{ y * right + x + top * right + left }; index < max_size) {
					const uint32_t alpha{ static_cast<uint32_t>(bitmap.buffer[y * bitmap.pitch + x]) << 24 };
					destination[index] |= alpha | color;
				}
			}
		}
	}

};

font::font(const std::string& path, int size) {
	ft::initialize();
	if (const auto final_path = find_absolute_path(path)) {
		face = new font_face{ final_path.value() };
		face->set_size(size);
	} else {
		WARNING("Did not find font: " << path);
	}
}

font::font(font&& that) noexcept {
	std::swap(face, that.face);
	std::swap(line_space, that.line_space);
}

font::~font() {
	delete face;
}

font& font::operator=(font&& that) noexcept {
	std::swap(face, that.face);
	std::swap(line_space, that.line_space);
	return *this;
}

void font::render(surface& surface, const std::string& text, uint32_t color) const {
	if (auto result = render_text(text, color); result.first) {
		surface.render(result.first, result.second.x, result.second.y);
		delete[] result.first;
	}
}

surface font::render(const std::string& text, uint32_t color) const {
	if (auto result = render_text(text, color); result.first) {
		return { result.first, result.second.x, result.second.y, pixel_format::rgba, surface::construct_by::move };
	} else {
		return { 2, 2, pixel_format::rgba };
	}
}

font::text_size font::size(const std::string& text) const {
	text_size text_size;
	if (!face) {
		return text_size;
	}
	int last_index{ -1 };
	size_t string_index{ 0 };
	int current_row_width{ 2 }; // should be 0. see below for left margin.
	text_size.size.x = 1;
	while (text.size() > string_index) {
		const uint32_t character{ utf8::next_character(text, &string_index) };
		if (character == unicode::byte_order_mark || character == unicode::byte_order_mark_swapped) {
			continue;
		}
		if (character == '\n' || character == '\r') {
			if (current_row_width > text_size.size.x) {
				text_size.size.x = current_row_width;
			}
			current_row_width = 2; // should be 0. see below for left margin.
			text_size.rows++;
			continue;
		}
		uint32_t index{ face->char_index(character) };
		if (face->has_kerning && last_index > 0 && index > 0) {
			FT_Vector delta;
			FT_Get_Kerning(face->face, last_index, index, FT_KERNING_DEFAULT, &delta);
			text_size.size.x += delta.x >> 6;
		}
		face->load_glyph(index);
		const auto glyph{ face->face->glyph };
		current_row_width += glyph->advance.x >> 6;
		const int glyph_max_y{ ft::font_to_pixel(ft::floor(glyph->metrics.horiBearingY)) };
		const int glyph_min_y{ glyph_max_y - ft::font_to_pixel(ft::ceil(glyph->metrics.height)) };
		if (glyph_min_y < text_size.min_y) {
			text_size.min_y = glyph_min_y;
		}
		if (glyph_max_y > text_size.max_y) {
			text_size.max_y = glyph_max_y;
		}
		text_size.size.y = std::max(text_size.size.y, static_cast<int>(ft::font_to_pixel(ft::ceil(glyph->metrics.height))));
		last_index = index;
	}
	if (current_row_width > text_size.size.x) {
		text_size.size.x = current_row_width;
	}
	text_size.size.y -= text_size.min_y;
	if (text_size.max_y > text_size.size.y) {
		text_size.size.y = text_size.max_y;
	}
	text_size.size.y *= static_cast<int>(static_cast<float>(text_size.rows)* line_space);
	return text_size;
}

bool font::exists(const std::string& path) {
	if (std::filesystem::exists(path)) {
		return true;
	}
	auto windows_font_path = platform::environment_variable("WINDIR") + "\\Fonts\\" + path;
	return std::filesystem::exists(windows_font_path);
}

std::optional<std::string> font::find_absolute_path(const std::string& relative_path) {
	if (std::filesystem::exists(relative_path)) {
		return relative_path;
	}
	auto path = "fonts/" + relative_path;
	if (std::filesystem::exists(path)) {
		return path;
	}
	path = platform::environment_variable("WINDIR") + "\\Fonts\\" + relative_path;
	return std::filesystem::exists(path) ? path : std::optional<std::string>{};
}

std::pair<uint32_t*, vector2i> font::render_text(const std::string& text, uint32_t color) const {
	if (!face) {
		return { nullptr, {} };
	}
	color &= 0x00FFFFFF; // alpha is added by freetype
	text_size text_size{ size(text) };
	auto destination = new uint32_t[text_size.size.x * text_size.size.y];
	std::fill_n(destination, text_size.size.x * text_size.size.y, 0x00000000);
	int left{ 2 }; // this should be 0, but fonts with negative margin on first letter is not supported
	// remember to also change to 0 below on newlines when this is fixed
	int last_index{ -1 };
	size_t string_index{ 0 };
	int row{ text_size.rows - 1 };
	while (text.size() > string_index) {
		const uint32_t character{ utf8::next_character(text, &string_index) };
		if (character == unicode::byte_order_mark || character == unicode::byte_order_mark_swapped) {
			continue;
		}
		if (character == '\n' || character == '\r') {
			left = 2; // this should also be 0, see above.
			row--;
			continue;
		}
		const uint32_t index{ face->char_index(character) };
		if (face->has_kerning && last_index > 0 && index > 0) {
			FT_Vector delta;
			FT_Get_Kerning(face->face, last_index, index, FT_KERNING_DEFAULT, &delta);
			left += delta.x >> 6;
		}
		face->load_glyph(index);
		face->render_glyph();
		const auto* glyph = face->face->glyph;
		const int row_y{ (text_size.size.y / text_size.rows) * row };
		const int top{ text_size.size.y - row_y - glyph->bitmap_top + text_size.min_y };
		face->blit(destination, left + glyph->bitmap_left, top, text_size.size.x, text_size.size.y, color);
		left += glyph->advance.x >> 6;
		last_index = index;
	}
	return { destination, text_size.size };
}

}
