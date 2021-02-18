module;

#include "assert.hpp"

export module nfwk.graphics:surface;

import std.core;
import std.filesystem;
import nfwk.core;

export namespace nfwk {

enum class pixel_format { rgba, bgra };

class surface {
public:

	enum class construct_by { copy, move };

	surface(std::uint32_t* source_pixels, int width, int height, pixel_format format, construct_by construction)
		: size{ width, height }, format_{ format } {
		ASSERT(source_pixels);
		switch (construction) {
		case construct_by::copy:
			pixels = new std::uint32_t[width * height];
			std::memcpy(pixels, source_pixels, width * height * sizeof(std::uint32_t));
			break;
		case construct_by::move:
			pixels = source_pixels;
			break;
		default:
			break;
		}
	}

	surface(int width, int height, pixel_format format) : size{ width, height }, format_{ format } {
		pixels = new std::uint32_t[width * height];
	}

	surface(int width, int height, pixel_format format, std::uint32_t color) : surface{ width, height, format } {
		std::fill_n(pixels, width * height, color);
	}

	surface() = default;
	surface(const surface&) = delete;

	surface(surface&& that) noexcept {
		std::swap(pixels, that.pixels);
		std::swap(size, that.size);
		std::swap(format_, that.format_);
	}

	~surface() {
		delete[] pixels;
	}

	surface& operator=(const surface&) = delete;

	surface& operator=(surface&& that) noexcept {
		std::swap(pixels, that.pixels);
		std::swap(size, that.size);
		std::swap(format_, that.format_);
		return *this;
	}

	std::uint32_t* data() const {
		return pixels;
	}

	std::uint32_t at(int x, int y) const {
		return pixels[y * size.x + x];
	}

	std::uint32_t at(int index) const {
		return pixels[index];
	}

	void set(int x, int y, std::uint32_t color) {
		pixels[y * size.x + x] = color;
	}

	void set(int index, std::uint32_t color) {
		pixels[index] = color;
	}

	void clear(std::uint32_t color) {
		std::fill_n(pixels, size.x * size.y, color);
	}

	vector2i dimensions() const {
		return size;
	}
	
	int width() const {
		return size.x;
	}
	
	int height() const {
		return size.y;
	}
	
	int count() const {
		return size.x * size.y;
	}

	pixel_format format() const {
		return format_;
	}

	void resize(int width, int height) {
		if (size.x >= width && size.y >= height) {
			size = { width, height };
			return;
		}
		std::uint32_t* old_pixels{ pixels };
		const int count{ width * height };
		pixels = new std::uint32_t[count];
		for (int y{ 0 }; y < height; y++) {
			std::memcpy(pixels + y * width, old_pixels + y * size.x, size.x * sizeof(uint32_t));
			std::memset(pixels + y * width + size.x, 0x00, width - size.x * sizeof(uint32_t));
		}
		size = { width, height };
		delete[] old_pixels;
	}

	void flip_frames_horizontally(int frames) {
		const int frame_width{ size.x / frames };
		const int frame_half_width{ frame_width / 2 - frame_width % 2 };
		int frame_begin_x{ 0 };
		for (int frame{ 0 }; frame < frames; frame++) {
			const int frame_middle_x{ frame_begin_x + frame_half_width };
			for (int x{ frame_begin_x }; x < frame_middle_x; x++) {
				for (int y{ 0 }; y < size.y; y++) {
					const int row{ y * size.x };
					const int right_column{ frame_begin_x + frame_width - (x - frame_begin_x) - 1 };
					const std::uint32_t left{ pixels[row + x] };
					pixels[row + x] = pixels[row + right_column];
					pixels[row + right_column] = left;
				}
			}
			frame_begin_x += frame_width;
		}
	}

	void flip_horizontally() {
		const int half_width{ size.x / 2 - size.x % 2 };
		for (int x{ 0 }; x < half_width; x++) {
			for (int y{ 0 }; y < size.y; y++) {
				const int row{ y * size.x };
				const std::uint32_t left{ pixels[row + x] };
				pixels[row + x] = pixels[row + size.x - x - 1];
				pixels[row + size.x - x - 1] = left;
			}
		}
	}

	void flip_vertically() {
		flip_vertically(pixels, size);
	}

	void render(std::uint32_t* source_pixels, int width, int height) {
		if (width > size.x || height > size.y) {
			resize(width, height);
		}
		if (size.x == width && size.y == height) {
			std::memcpy(pixels, source_pixels, width * height * sizeof(std::uint32_t));
			return;
		}
		for (int y{ 0 }; y < height; y++) {
			std::memcpy(pixels + y * size.x, source_pixels + y * width, width * sizeof(std::uint32_t));
		}
	}

	void render_horizontal_line(std::uint32_t color, int x, int y, int width) {
		std::fill_n(pixels + y * size.x + x, width, color);
	}

	void render_vertical_line(uint32_t color, int x, int y, int height) {
		const int y1{ y * size.x + x };
		const int y2{ (y + height) * size.x + x };
		for (int i{ y1 }; i < y2; i += size.x) {
			pixels[i] = color;
		}
	}

	void render_rectangle(uint32_t color, int x, int y, int width, int height) {
		const int y1{ y * size.x + x };
		const int y2{ (y + height) * size.x + x };
		for (int i{ y1 }; i < y2; i += size.x) {
			std::fill_n(pixels + i, width, color);
		}
	}

	void render_circle(std::uint32_t color, int center_x, int center_y, int radius) {
		ASSERT(center_x - radius >= 0);
		ASSERT(center_y - radius >= 0);
		ASSERT(center_x + radius < size.x);
		ASSERT(center_y + radius < size.y);
		const int diameter{ radius * 2 };
		int x{ radius - 1 };
		int y{ 0 };
		int dx{ 1 };
		int dy{ 1 };
		int err{ dx - diameter };
		while (x >= y) {
			std::fill_n(pixels + (center_y + y) * size.x + center_x - x, x * 2, color);
			std::fill_n(pixels + (center_y + x) * size.x + center_x - y, y * 2, color);
			std::fill_n(pixels + (center_y - y) * size.x + center_x - x, x * 2, color);
			std::fill_n(pixels + (center_y - x) * size.x + center_x - y, y * 2, color);
			if (err <= 0) {
				y++;
				err += dy;
				dy += 2;
			}
			if (err > 0) {
				x--;
				dx += 2;
				err += dx - diameter;
			}
		}
	}
	
	static void flip_vertically(std::uint32_t* pixels, vector2i size) {
		const int half_height{ size.y / 2 };
		auto temporary_rows = new std::uint32_t[size.x * sizeof(std::uint32_t)];
		for (int y{ 0 }; y < half_height; y++) {
			std::memcpy(temporary_rows, pixels + y * size.x, size.x * sizeof(std::uint32_t));
			std::memcpy(pixels + y * size.x, pixels + (size.y - y - 1) * size.x, size.x * sizeof(std::uint32_t));
			std::memcpy(pixels + (size.y - y - 1) * size.x, temporary_rows, size.x * sizeof(std::uint32_t));
		}
		delete[] temporary_rows;
	}

private:

	std::uint32_t* pixels{ nullptr };
	vector2i size;
	pixel_format format_{ pixel_format::rgba };

};

}
