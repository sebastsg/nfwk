#pragma once

#include "vector2.hpp"

#include <filesystem>

namespace no {

enum class pixel_format { rgba, bgra };

class surface {
public:

	enum class construct_by { copy, move };

	static void flip_vertically(uint32_t* pixels, vector2i size);

	surface(const std::filesystem::path& path);
	surface(uint32_t* pixels, int width, int height, pixel_format format, construct_by construction);
	surface(int width, int height, pixel_format format);
	surface(int width, int height, pixel_format format, uint32_t color);
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

	uint32_t* data() const;
	uint32_t at(int x, int y) const;
	uint32_t at(int index) const;
	void set(int x, int y, uint32_t color);
	void set(int index, uint32_t color);

	void clear(uint32_t color);
	void render(uint32_t* pixels, int width, int height);
	void render_horizontal_line(uint32_t color, int x, int y, int width);
	void render_vertical_line(uint32_t color, int x, int y, int height);
	void render_rectangle(uint32_t color, int x, int y, int width, int height);
	void render_circle(uint32_t color, int x, int y, int radius);

	vector2i dimensions() const;
	int width() const;
	int height() const;
	int count() const;
	pixel_format format() const;

private:

	uint32_t* pixels{ nullptr };
	vector2i size;
	pixel_format format_{ pixel_format::rgba };

};

}
