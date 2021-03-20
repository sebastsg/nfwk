#pragma once

#include "vector4.hpp"

namespace nfwk {

std::uint32_t to_rgba(const vector3f& color);
std::uint32_t to_rgba(const vector4f& color);
std::uint32_t to_rgba(const vector3i& color);
std::uint32_t to_rgba(const vector4i& color);

}
