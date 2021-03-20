#pragma once

#include "vector2.hpp"

#include <filesystem>

namespace nfwk {

enum class pixel_format { rgba, bgra };

class surface {
public:

	enum class construct_by { copy, move };

	static void flip_vertically(std::uint32_t* pixels, vector2i size);

	surface(const std::filesystem::path& path);
	surface(std::uint32_t* pixels, int width, int height, pixel_format format, construct_by construction);
	surface(int width, int height, pixel_format format);
	surface(int width, int height, pixel_format format, std::uint32_t color);
	surface() = default;
	surface(const surface&) = delete;
	surface(surface&&) noexcept;
	~surface();

	surface& operator=(const surface&) = delete;
	surface& operator=(surface&&) noexcept;

	void resize(int width, int height);

	void flip_frames_horizontally(int frames);
	void flip_horizontally();
	void flip_vertically();

	std::uint32_t* data() const;
	std::uint32_t at(int x, int y) const;
	std::uint32_t at(int index) const;
	void set(int x, int y, std::uint32_t color);
	void set(int index, std::uint32_t color);

	void clear(std::uint32_t color);
	void render(std::uint32_t* pixels, int width, int height);
	void render_horizontal_line(std::uint32_t color, int x, int y, int width);
	void render_vertical_line(std::uint32_t color, int x, int y, int height);
	void render_rectangle(std::uint32_t color, int x, int y, int width, int height);
	void render_circle(std::uint32_t color, int x, int y, int radius);

	vector2i dimensions() const;
	int width() const;
	int height() const;
	int count() const;
	pixel_format format() const;

private:

	std::uint32_t* pixels{ nullptr };
	vector2i size;
	pixel_format format_{ pixel_format::rgba };

};

}
