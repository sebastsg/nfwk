#include "graphics/window.hpp"
#include "loop.hpp"
#include "subprogram.hpp"
#include "log.hpp"

std::ostream& operator<<(std::ostream& out, nfwk::window::display_mode mode) {
	switch (mode) {
	case nfwk::window::display_mode::windowed: return out << "Windowed";
	case nfwk::window::display_mode::fullscreen: return out << "Fullscreen";
	case nfwk::window::display_mode::fullscreen_desktop: return out << "Fullscreen Desktop";
	}
}

namespace nfwk {

thread_local render_context* render_context::current_context{ nullptr };
thread_local window* render_context::current_window{ nullptr };

render_context* render_context::get_current_context() {
	return current_context;
}

window* render_context::get_current_window() {
	return current_window;
}

window::window() {
	close_event_listener = on_close.listen([this] {
		info(draw::log, "About to close window: {}.", title());
		for (auto& subprogram : attached_subprograms) {
			subprogram->stop();
		}
		attached_subprograms.clear();
	});
	draw_event_listener = on_draw_end.listen([this] {
		swap();
	});
}

window::~window() {
	
}

void window::attach_subprogram(subprogram& subprogram) {
	attached_subprograms.push_back(&subprogram);
}

bool window::has_attached_subprogram(const subprogram& subprogram) const {
	for (const auto& attached_subprogram : attached_subprograms) {
		if (attached_subprogram == &subprogram) {
			return true;
		}
	}
	return false;
}

}
