#pragma once

#include "log.hpp"
#include "loop.hpp"

namespace nfwk::debug::menu {

void add(std::u8string_view id, std::u8string_view name, const std::function<void()>& update);
void add(std::u8string_view id, const std::function<void()>& update);
void enable(loop& loop);
void disable();
void update();
void remove(std::u8string_view id);

}
