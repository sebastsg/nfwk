#pragma once

#include "vector4.hpp"

namespace nfwk {

std::uint32_t to_rgba(const vector3f& normalized_0_1);
std::uint32_t to_rgba(const vector4f& normalized_0_1);
std::uint32_t f255_to_rgba(const vector3f& color_0_255);
std::uint32_t to_rgba(const vector3i& color_0_255);
std::uint32_t to_rgba(const vector4i& color_0_255);

vector4i from_rgba(std::uint32_t color);

}
