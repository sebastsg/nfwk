#pragma once

namespace no {

class window;

namespace ui {

void create(window& window);
void destroy();
void start_frame();
void end_frame();
void draw();

}

}
