#include "jpeg.hpp"
#include "log.hpp"

#include "libjpeg-turbo/turbojpeg.h"

namespace nfwk {

static std::string get_tj_error_message(tjhandle handle) {
	const auto* message = tjGetErrorStr2(handle);
	return std::format("Error: {}. Message: {}", tjGetErrorCode(handle), message ? message : "null");
}

surface load_jpeg(const std::filesystem::path& path) {
	io_stream compressed_stream;
	read_file(path, compressed_stream);
	if (compressed_stream.size_left_to_read() == 0) {
		warning("graphics", "JPEG file does not exist: {}", path);
		return { 2, 2, pixel_format::rgba };
	}
	auto compressed_read = reinterpret_cast<unsigned char*>(compressed_stream.at_read());
	const auto handle = tjInitDecompress();
	int width{ 0 };
	int height{ 0 };
	int subsampling{ 0 };
	int color_space{ 0 };
	if (tjDecompressHeader3(handle, compressed_read, compressed_stream.size_left_to_read(), &width, &height, &subsampling, &color_space) == -1) {
		warning("graphics", "Failed to decompress header for {}. {}", path, get_tj_error_message(handle));
		tjDestroy(handle);
		return { 2, 2, pixel_format::rgba };
	}

	io_stream decompressed_stream{ static_cast<std::size_t>(width) * height * 4 };
	auto decompressed_write = reinterpret_cast<unsigned char*>(decompressed_stream.at_write());

	constexpr int pitch{ 0 }; // 0 => scaled_width * tjPixelSize[pixel_format]
	if (tjDecompress2(handle, compressed_read, compressed_stream.size_left_to_read(), decompressed_write, width, pitch, height, TJPF_RGBA, TJFLAG_FASTDCT) == -1) {
		warning("graphics", "Failed to decompress {}. {}", path, get_tj_error_message(handle));
		tjDestroy(handle);
		return { 2, 2, pixel_format::rgba };
	}

	tjDestroy(handle);
	return { reinterpret_cast<std::uint32_t*>(decompressed_stream.at_read()), width, height, pixel_format::rgba, surface::construct_by::copy };
}

}
