#include "png.hpp"
#include "debug.hpp"

#include <libpng/png.h>

#include <cerrno>
#include <filesystem>

namespace no {

surface load_png(const std::string& path) {
	if (!std::filesystem::is_regular_file(path) || std::filesystem::path{ path }.extension() != ".png") {
		return { 2, 2, pixel_format::rgba };
	}
	png_structp png{ png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr) };
	if (!png) {
		WARNING_X("graphics", "Failed to create read structure");
		return { 2, 2, pixel_format::rgba };
	}
	png_infop info{ png_create_info_struct(png) };
	if (!info) {
		WARNING_X("graphics", "Failed to create info structure");
		return { 2, 2, pixel_format::rgba };
	}
	if (setjmp(png_jmpbuf(png))) {
		WARNING_X("graphics", "Failed to load image: " << path);
		return { 2, 2, pixel_format::rgba };
	}
#if PLATFORM_WINDOWS
	FILE* file{ nullptr };
	const errno_t error{ fopen_s(&file, path.c_str(), "rb") };
#else
	FILE* file{ fopen(path.c_str(), "rb") };
	const int error{ errno };
#endif
	if (error == ENOENT) {
		WARNING_X("graphics", "Image file was not found: " << path);
		return { 2, 2, pixel_format::rgba };
	}

	png_init_io(png, file);
	png_read_info(png, info);

	const uint32_t width{ png_get_image_width(png, info) };
	const uint32_t height{ png_get_image_height(png, info) };
	const uint8_t color_type{ png_get_color_type(png, info) };
	const uint8_t bit_depth{ png_get_bit_depth(png, info) };

	if (bit_depth == 16) {
		png_set_strip_16(png);
	}
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png);
	}
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
		png_set_expand_gray_1_2_4_to_8(png);
	}
	if (png_get_valid(png, info, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png);
	}
	switch (color_type) {
	case PNG_COLOR_TYPE_RGB:
	case PNG_COLOR_TYPE_GRAY:
	case PNG_COLOR_TYPE_PALETTE:
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	}
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png);
	}
	png_read_update_info(png, info);

	auto rows = new uint8_t* [height];
	const size_t row_size{ png_get_rowbytes(png, info) };
	for (uint32_t y{ 0 }; y < height; y++) {
		rows[y] = new uint8_t[row_size];
	}
	png_read_image(png, rows);
	fclose(file);
	png_destroy_read_struct(&png, &info, nullptr);

	uint32_t* pixels = new uint32_t[width * height];
	for (uint32_t y{ 0 }; y < height; y++) {
		auto destination = reinterpret_cast<uint8_t*>(pixels + y * width);
		memcpy(destination, rows[y], row_size);
		delete[] rows[y];
	}
	delete[] rows;
	MESSAGE_X("graphics", "Loaded PNG file " << path << ". Size: " << width << ", " << height);
	return { pixels, static_cast<int>(width), static_cast<int>(height), pixel_format::rgba, surface::construct_by::move };
}

}
