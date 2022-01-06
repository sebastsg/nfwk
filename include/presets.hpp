#pragma once

#include "assets.hpp"
#include "graphics/texture.hpp"
#include "graphics/font.hpp"
#include "graphics/shader.hpp"
#include "graphics/window.hpp"
#include "imgui_loop_component.hpp"
#include "loop.hpp"

namespace nfwk {

window_manager& get_window_manager(subprogram& subprogram);

std::unique_ptr<asset_manager> make_asset_manager(const std::filesystem::path& directory);
std::shared_ptr<window> make_window(subprogram& subprogram, std::string_view title, std::optional<vector2i> size = std::nullopt);

}
