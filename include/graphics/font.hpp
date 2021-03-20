#pragma once

#include "surface.hpp"

#include <optional>
#include <filesystem>

namespace nfwk {

class font {
public:

	struct text_size {
		vector2i size;
		int min_y{ 0 };
		int max_y{ 0 };
		int rows{ 1 };
	};

	font(const std::filesystem::path& path, int size);
	font(const font&) = delete;
	font(font&&) = delete;

	~font();

	font& operator=(const font&) = delete;
	font& operator=(font&&) = delete;

	void render(surface& surface, std::string_view text, std::uint32_t color = 0x00ffffff) const;
	surface render(std::string_view text, std::uint32_t color = 0x00ffffff) const;
	text_size size(std::string_view text) const;

	static bool exists(const std::filesystem::path& path);
	static std::optional<std::filesystem::path> find_absolute_path(const std::filesystem::path& path);

private:

	class font_face;

	std::pair<std::uint32_t*, vector2i> render_text(std::string_view text, std::uint32_t color) const;

	std::unique_ptr<font_face> face;
	float line_space{ 1.25f };

};

}
