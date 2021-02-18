module;

#include <ft2build.h>
#include FT_FREETYPE_H

#include "assert.hpp"

export module nfwk.graphics:font;

import std.core;
import std.memory;
import std.filesystem;
import nfwk.core;
import :surface;
import :font_face;

export namespace nfwk {

class font {
public:

	struct text_size {
		vector2i size;
		int min_y{ 0 };
		int max_y{ 0 };
		int rows{ 1 };
	};

	font() = default;

	font(const std::string& path, int size) {
		ft::initialize();
		if (const auto final_path = find_absolute_path(path)) {
			face = std::make_unique<font_face>(final_path.value());
			face->set_size(size);
		} else {
			warning("graphics", "Did not find font: {}", path);
		}
	}

	font(const font&) = delete;

	font(font&& that) noexcept {
		std::swap(face, that.face);
		std::swap(line_space, that.line_space);
	}

	~font() = default;

	font& operator=(const font&) = delete;

	font& operator=(font&& that) noexcept {
		std::swap(face, that.face);
		std::swap(line_space, that.line_space);
		return *this;
	}

	void render(surface& surface, const std::string& text, std::uint32_t color = 0x00ffffff) const {
		if (auto result = render_text(text, color); result.first) {
			surface.render(result.first, result.second.x, result.second.y);
			delete[] result.first;
		}
	}

	surface render(const std::string& text, std::uint32_t color = 0x00ffffff) const {
		if (const auto result = render_text(text, color); result.first) {
			return { result.first, result.second.x, result.second.y, pixel_format::rgba, surface::construct_by::move };
		} else {
			return { 2, 2, pixel_format::rgba };
		}
	}

	text_size size(const std::string& text) const {
		text_size text_size;
		if (!face) {
			return text_size;
		}
		int last_index{ -1 };
		std::size_t string_index{ 0 };
		int current_row_width{ 2 }; // should be 0. see below for left margin.
		text_size.size.x = 1;
		while (text.size() > string_index) {
			const std::uint32_t character{ utf8::next_character(text, &string_index) };
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
			std::uint32_t index{ face->char_index(character) };
			if (face->has_kerning && last_index > 0 && index > 0) {
				FT_Vector delta;
				FT_Get_Kerning(face->face, last_index, index, FT_KERNING_DEFAULT, &delta);
				text_size.size.x += delta.x >> 6;
			}
			face->load_glyph(index);
			const auto glyph = face->face->glyph;
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
		text_size.size.y *= static_cast<int>(static_cast<float>(text_size.rows) * line_space);
		return text_size;
	}

	static bool exists(const std::string& path) {
		if (std::filesystem::exists(path)) {
			return true;
		}
		auto windows_font_path = environment_variable("WINDIR") + "\\Fonts\\" + path;
		return std::filesystem::exists(windows_font_path);
	}

	static std::optional<std::string> find_absolute_path(const std::string& relative_path) {
		if (std::filesystem::exists(relative_path)) {
			return relative_path;
		}
		auto path = "fonts/" + relative_path;
		if (std::filesystem::exists(path)) {
			return path;
		}
		path = environment_variable("WINDIR") + "\\Fonts\\" + relative_path;
		return std::filesystem::exists(path) ? path : std::optional<std::string>{};
	}

private:

	std::unique_ptr<font_face> face;
	float line_space{ 1.25f };

	std::pair<std::uint32_t*, vector2i> render_text(const std::string& text, std::uint32_t color) const {
		if (!face) {
			return { nullptr, {} };
		}
		color &= 0x00ffffff; // alpha is added by freetype
		text_size text_size{ size(text) };
		auto destination = new std::uint32_t[text_size.size.x * text_size.size.y];
		std::fill_n(destination, text_size.size.x * text_size.size.y, 0x00000000);
		int left{ 2 }; // this should be 0, but fonts with negative margin on first letter is not supported
		// remember to also change to 0 below on newlines when this is fixed
		int last_index{ -1 };
		std::size_t string_index{ 0 };
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
			const std::uint32_t index{ face->char_index(character) };
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

};

}
