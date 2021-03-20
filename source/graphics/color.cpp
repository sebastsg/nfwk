#include "graphics/color.hpp"

namespace nfwk {

std::uint32_t to_rgba(const vector3f& normalized) {
	const vector3<std::uint32_t> color{ (normalized * 255.0f).to<std::uint32_t>() };
	return color.x | (color.y << 8) | (color.z << 16) | 0xff000000;
}

std::uint32_t to_rgba(const vector4f& normalized) {
	const vector4<std::uint32_t> color{ (normalized * 255.0f).to<std::uint32_t>() };
	return color.x | (color.y << 8) | (color.z << 16) | (color.w << 24);
}

std::uint32_t to_rgba(const vector3i& color) {
	return color.x | (color.y << 8) | (color.z << 16) | 0xff000000;
}

std::uint32_t to_rgba(const vector4i& color) {
	return color.x | (color.y << 8) | (color.z << 16) | (color.w << 24);
}

}
