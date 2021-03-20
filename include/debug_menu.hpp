#pragma once

#include "graphics/ui.hpp"
#include "graphics/draw.hpp"
#include "log.hpp"
#include "loop.hpp"

namespace nfwk::debug::menu {

void add(std::string_view id, std::string_view name, const std::function<void()>& update);
void add(std::string_view id, const std::function<void()>& update);
void enable(loop& loop);
void disable();
void update();
void remove(std::string_view id);

}
