#include "png.hpp"
#include "log.hpp"

#include <libpng/png.h>

#include <cerrno>

namespace nfwk {

surface load_png(const std::filesystem::path& path) {
	if (!std::filesystem::is_regular_file(path) || path.extension() != ".png") {
		return { 2, 2, pixel_format::rgba };
	}
	png_structp png{ png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr) };
	if (!png) {
		warning(graphics::log, "Failed to create read structure");
		return { 2, 2, pixel_format::rgba };
	}
	png_infop info{ png_create_info_struct(png) };
	if (!info) {
		warning(graphics::log, "Failed to create info structure");
		return { 2, 2, pixel_format::rgba };
	}
	if (setjmp(png_jmpbuf(png))) {
		warning(graphics::log, "Failed to load image: {}", path);
		return { 2, 2, pixel_format::rgba };
	}
#if 1
	FILE* file{ nullptr };
	const auto& path_string = path.wstring();
	const auto error = _wfopen_s(&file, path_string.c_str(), L"rb");
	if (!file) {
		return { 2, 2, pixel_format::rgba };
	}
#else
	FILE* file{ fopen(path.c_str(), "rb") };
	const int error{ errno };
#endif
	if (error == ENOENT) {
		warning(graphics::log, "Image file was not found: {}", path);
		return { 2, 2, pixel_format::rgba };
	}

	png_init_io(png, file);
	png_read_info(png, info);

	const std::uint32_t width{ png_get_image_width(png, info) };
	const std::uint32_t height{ png_get_image_height(png, info) };
	const std::uint8_t color_type{ png_get_color_type(png, info) };
	const std::uint8_t bit_depth{ png_get_bit_depth(png, info) };

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

	auto rows = new std::uint8_t* [height];
	const std::size_t row_size{ png_get_rowbytes(png, info) };
	for (std::uint32_t y{ 0 }; y < height; y++) {
		rows[y] = new std::uint8_t[row_size];
	}
	png_read_image(png, rows);
	fclose(file);
	png_destroy_read_struct(&png, &info, nullptr);

	auto pixels = new std::uint32_t[width * height];
	for (std::uint32_t y{ 0 }; y < height; y++) {
		auto destination = reinterpret_cast<std::uint8_t*>(pixels + y * width);
		memcpy(destination, rows[y], row_size);
		delete[] rows[y];
	}
	delete[] rows;
	message(graphics::log, "Loaded PNG file {}. Size: {}, {}", path, width, height);
	return { pixels, static_cast<int>(width), static_cast<int>(height), pixel_format::rgba, surface::construct_by::move };
}

}
