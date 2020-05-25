#pragma once

#include <optional>
#include <string>

namespace no {

class window;

namespace ui {

void create(window& window, std::optional<std::string> font_name, int font_size = 16);
void destroy();
void start_frame();
void end_frame();
void draw();

}

}
