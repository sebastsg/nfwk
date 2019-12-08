#pragma once

#include "math.hpp"

namespace no {

uint32_t to_rgba(const vector3f& color);
uint32_t to_rgba(const vector4f& color);
uint32_t to_rgba(const vector3i& color);
uint32_t to_rgba(const vector4i& color);

}
