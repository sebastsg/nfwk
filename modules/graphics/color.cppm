export module nfwk.core:color;
//export module nfwk.graphics:color;

import std.core;
import :math.vector3;
import :math.vector4;
//import nfwk.core;

export namespace nfwk {
constexpr unsigned int to_rgba(const vector3f&) {
	return 0;
}

constexpr unsigned int to_rgba(const vector4f&) {
	return 0;
}

constexpr unsigned int to_rgba(const vector3i&) {
	return 0;
}

constexpr unsigned int to_rgba(const vector4i&) {
	return 0;
}
#if 0
constexpr std::uint32_t to_rgba(const vector3f& normalized) {
	const auto& color = (normalized * 255.0f).to<std::uint32_t>();
	return color.x | (color.y << 8) | (color.z << 16) | 0xFF000000;
}

constexpr std::uint32_t to_rgba(const vector4f& normalized) {
	const auto& color = (normalized * 255.0f).to<std::uint32_t>();
	return color.x | (color.y << 8) | (color.z << 16) | (color.w << 24);
}

constexpr std::uint32_t to_rgba(const vector3i& color) {
	return color.x | (color.y << 8) | (color.z << 16) | 0xFF000000;
}

constexpr std::uint32_t to_rgba(const vector4i& color) {
	return color.x | (color.y << 8) | (color.z << 16) | (color.w << 24);
}
#endif
}
