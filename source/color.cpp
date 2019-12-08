#include "color.hpp"

namespace no {

uint32_t to_rgba(const vector3f& normalized) {
	const vector3<uint32_t> color{ (normalized * 255.0f).to<uint32_t>() };
	return color.x | (color.y << 8) | (color.z << 16) | 0xFF000000;
}

uint32_t to_rgba(const no::vector4f& normalized) {
	const vector4<uint32_t> color{ (normalized * 255.0f).to<uint32_t>() };
	return color.x | (color.y << 8) | (color.z << 16) | (color.w << 24);
}

uint32_t to_rgba(const vector3i& color) {
	return color.x | (color.y << 8) | (color.z << 16) | 0xFF000000;
}

uint32_t to_rgba(const vector4i& color) {
	return color.x | (color.y << 8) | (color.z << 16) | (color.w << 24);
}

}
