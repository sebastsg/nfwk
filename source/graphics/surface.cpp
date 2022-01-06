#include "graphics/surface.hpp"
#include "graphics/png.hpp"
#include "graphics/jpeg.hpp"
#include "graphics/color.hpp"
#include "log.hpp"
#include "assert.hpp"
#include "math.hpp"

namespace nfwk {

surface::surface(surface&& that) noexcept {
	std::swap(pixels, that.pixels);
	std::swap(size, that.size);
	std::swap(format_, that.format_);
	std::swap(error_flag, that.error_flag);
	//std::swap(on_changed, that.on_changed);
}

surface::surface(const std::filesystem::path& path) {
	const auto type = detect_file_type_from_extension(path_to_string(path.extension()));
	if (!type.has_value()) {
		warning("graphics", "Unknown image file type: {}", path);
		error_flag = true;
		return;
	}
	const auto initialize_from = [this](surface that) {
		pixels = that.pixels;
		size = that.size;
		format_ = that.format_;
		that.pixels = nullptr;
	};
	switch (type.value()) {
	case file_type::png:
		initialize_from(load_png(path));
		break;
	case file_type::jpg:
		initialize_from(load_jpeg(path));
		break;
	default:
		warning("graphics", "No decoder for file: {} (type: {})", path, enum_string(type));
		size = { 2, 2 };
		pixels = new std::uint32_t[4]{};
		error_flag = true;
		break;
	}
}

surface::surface(std::uint32_t* source_pixels, int width, int height, pixel_format format, construct_by construction) : size{ width, height }, format_{ format } {
	ASSERT(source_pixels);
	switch (construction) {
	case construct_by::copy:
		pixels = new std::uint32_t[width * height];
		std::memcpy(pixels, source_pixels, width * height * sizeof(std::uint32_t));
		break;
	case construct_by::move:
		pixels = source_pixels;
		break;
	}
}

surface::surface(int width, int height, pixel_format format) : size{ width, height }, format_{ format } {
	pixels = new std::uint32_t[width * height];
}

surface::surface(int width, int height, pixel_format format, std::uint32_t color) : surface{ width, height, format } {
	std::fill_n(pixels, width * height, color);
}

surface::~surface() {
	delete[] pixels;
}

surface& surface::operator=(surface&& that) noexcept {
	std::swap(pixels, that.pixels);
	std::swap(size, that.size);
	std::swap(format_, that.format_);
	std::swap(error_flag, that.error_flag);
	//std::swap(on_changed, that.on_changed);
	return *this;
}

void surface::resize(int width, int height) {
	if (size.x >= width && size.y >= height) {
		size = { width, height };
		return;
	}
	ASSERT(pixels);
	std::uint32_t* old_pixels{ pixels };
	const int count{ width * height };
	pixels = new std::uint32_t[count];
	for (int y{ 0 }; y < height; y++) {
		std::memcpy(pixels + y * width, old_pixels + y * size.x, size.x * sizeof(std::uint32_t));
		std::memset(pixels + y * width + size.x, 0x00, width - size.x * sizeof(std::uint32_t));
	}
	size = { width, height };
	delete[] old_pixels;
}

void surface::flip_frames_horizontally(int frames) {
	ASSERT(pixels);
	const auto frame_width = size.x / frames;
	const auto frame_half_width = frame_width / 2 - frame_width % 2;
	int frame_begin_x{ 0 };
	for (int frame{ 0 }; frame < frames; frame++) {
		const auto frame_middle_x = frame_begin_x + frame_half_width;
		for (int x{ frame_begin_x }; x < frame_middle_x; x++) {
			for (int y{ 0 }; y < size.y; y++) {
				const auto row = y * size.x;
				const auto right_column = frame_begin_x + frame_width - (x - frame_begin_x) - 1;
				const auto left = pixels[row + x];
				pixels[row + x] = pixels[row + right_column];
				pixels[row + right_column] = left;
			}
		}
		frame_begin_x += frame_width;
	}
}

void surface::flip_horizontally() {
	ASSERT(pixels);
	const auto half_width = size.x / 2 - size.x % 2;
	for (int x{ 0 }; x < half_width; x++) {
		for (int y{ 0 }; y < size.y; y++) {
			const auto row = y * size.x;
			const auto left = pixels[row + x];
			pixels[row + x] = pixels[row + size.x - x - 1];
			pixels[row + size.x - x - 1] = left;
		}
	}
}

void surface::flip_vertically() {
	flip_vertically(pixels, size);
}

std::uint32_t* surface::data() const {
	return pixels;
}

std::uint32_t surface::at(int x, int y) const {
	return pixels[y * size.x + x];
}

std::uint32_t surface::at(int index) const {
	return pixels[index];
}

void surface::set(int x, int y, std::uint32_t color) {
	pixels[y * size.x + x] = color;
}

void surface::set(int index, std::uint32_t color) {
	pixels[index] = color;
}

void surface::clear(std::uint32_t color) {
	ASSERT(pixels);
	std::fill_n(pixels, size.x * size.y, color);
}

void surface::render(std::uint32_t* source_pixels, int width, int height) {
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

void surface::render_horizontal_line(std::uint32_t color, int x, int y, int width) {
	ASSERT(pixels);
	std::fill_n(pixels + y * size.x + x, width, color);
}

void surface::render_vertical_line(std::uint32_t color, int x, int y, int height) {
	ASSERT(pixels);
	const int y1{ y * size.x + x };
	const int y2{ (y + height) * size.x + x };
	for (int i{ y1 }; i < y2; i += size.x) {
		pixels[i] = color;
	}
}

void surface::render_rectangle(std::uint32_t color, int x, int y, int width, int height) {
	ASSERT(pixels);
	const int y1{ y * size.x + x };
	const int y2{ (y + height) * size.x + x };
	for (int i{ y1 }; i < y2; i += size.x) {
		std::fill_n(pixels + i, width, color);
	}
}

void surface::render_circle(std::uint32_t color, int center_x, int center_y, int radius) {
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

vector2i surface::dimensions() const {
	return size;
}

int surface::width() const {
	return size.x;
}

int surface::height() const {
	return size.y;
}

int surface::count() const {
	return size.x * size.y;
}

pixel_format surface::format() const {
	return format_;
}

bool surface::has_error() const {
	return error_flag;
}

void surface::clear_error() {
	error_flag = false;
}

void surface::flip_vertically(std::uint32_t* pixels, vector2i size) {
	ASSERT(pixels);
	const int half_height{ size.y / 2 };
	std::uint32_t* temporary_rows{ new std::uint32_t[size.x * sizeof(std::uint32_t)] };
	for (int y{ 0 }; y < half_height; y++) {
		std::memcpy(temporary_rows, pixels + y * size.x, size.x * sizeof(std::uint32_t));
		std::memcpy(pixels + y * size.x, pixels + (size.y - y - 1) * size.x, size.x * sizeof(std::uint32_t));
		std::memcpy(pixels + (size.y - y - 1) * size.x, temporary_rows, size.x * sizeof(std::uint32_t));
	}
	delete[] temporary_rows;
}

float detect_grayscale(const surface& surface, float coverage, float flexibility) {
	ASSERT(coverage > 0.0f && coverage <= 1.0f);
	ASSERT(flexibility >= 0.0f && flexibility < 1.0f);
	const auto flexibility_distance = static_cast<int>(flexibility * 255.0f);
	const auto count = surface.count();
	const auto pixels_covered = static_cast<float>(count) * coverage;
	const auto skip = count / static_cast<int>(pixels_covered);
	const auto confidence_delta = 1.0f / static_cast<float>(pixels_covered);
	float confidence{ 0.0f };
	for (int i{ 0 }; i < count; i += skip) {
		const auto pixel = from_rgba(surface.at(i));
		if (pixel.x == pixel.y && pixel.y == pixel.z) {
			confidence += confidence_delta;
		} else {
			const auto xy_half = (pixel.x + pixel.y) / 2;
			const auto xz_half = (pixel.x + pixel.z) / 2;
			const auto yz_half = (pixel.y + pixel.z) / 2;
			if (std::abs(pixel.x - yz_half) < flexibility_distance && std::abs(pixel.y - xz_half) < flexibility_distance && std::abs(pixel.z - xy_half) < flexibility_distance) {
				confidence += confidence_delta * 0.5f;
			} else {
				confidence -= confidence_delta * 2.0f;
			}
		}
	}
	return clamp(confidence, 0.0f, 1.0f);
}

}
