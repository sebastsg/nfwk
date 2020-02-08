#pragma once

#include "math.hpp"

#include <optional>

namespace no {

class ortho_camera;

void create_grid(std::optional<int> texture = std::nullopt);
void destroy_grid();
void draw_grid(const ortho_camera& camera, vector2f size);

}
