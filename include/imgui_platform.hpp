#pragma once

#include <filesystem>

namespace nfwk {
class window;
class render_context;
}

namespace nfwk::ui {

void create(nfwk::window& window);
void destroy();
void start_frame();
void end_frame();
void draw(render_context& context);

void add_font(const std::filesystem::path& path, int size);
void build_fonts();

}
