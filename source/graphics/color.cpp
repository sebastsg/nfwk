#include "graphics/color.hpp"

namespace nfwk {

std::uint32_t to_rgba(const vector3f& normalized_0_1) {
	const auto color = (normalized_0_1 * 255.0f).to<std::uint32_t>();
	return color.x | (color.y << 8) | (color.z << 16) | 0xff000000;
}

std::uint32_t to_rgba(const vector4f& normalized_0_1) {
	const auto color = (normalized_0_1 * 255.0f).to<std::uint32_t>();
	return color.x | (color.y << 8) | (color.z << 16) | (color.w << 24);
}

std::uint32_t f255_to_rgba(const vector3f& color_0_255) {
	const auto color = color_0_255.to<std::uint32_t>();
	return color.x | (color.y << 8) | (color.z << 16) | 0xff000000;
}

std::uint32_t to_rgba(const vector3i& color_0_255) {
	return color_0_255.x | (color_0_255.y << 8) | (color_0_255.z << 16) | 0xff000000;
}

std::uint32_t to_rgba(const vector4i& color_0_255) {
	return color_0_255.x | (color_0_255.y << 8) | (color_0_255.z << 16) | (color_0_255.w << 24);
}

vector4i from_rgba(std::uint32_t color) {
	return {
		static_cast<int>(color & 0xff),
		static_cast<int>((color >> 8) & 0xff),
		static_cast<int>((color >> 16) & 0xff),
		static_cast<int>((color >> 24) & 0xff)
	};
}

}
