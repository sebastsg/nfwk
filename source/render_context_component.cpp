#include "imgui_loop_component.hpp"
#include "graphics/gl/wgl_context.hpp"
#include "graphics/windows_window.hpp"
#include "audio/wasapi.hpp"
#include "debug_menu.hpp"

namespace nfwk {

window_manager::window_manager(loop& loop, bool support_imgui) : owning_loop{ loop }, support_imgui{ support_imgui } {
	platform::windows_window::create_classes();
	frame_event_listener = owning_loop.on_begin_frame.listen([this] {
		for (const auto& window : windows) {
			window->on_draw_begin.emit();
			window->on_draw.emit();
			window->on_draw_end.emit();
		}
	});
}

std::shared_ptr<window> window_manager::create_window(std::u8string_view title, std::optional<vector2i> size) {
	auto compatibility_context = context ? nullptr : platform::windows_window::create_compatibility_render_context();
	auto& window = windows.emplace_back(std::make_shared<platform::windows_window>(title, size));
	if (!context) {
		context = window->create_render_context(std::nullopt);
		context->set_clear_color(0.1f);
		context->log_info();
		if (support_imgui) {
			imgui = std::make_unique<imgui_instance>(owning_loop, *window, *context);
		}
		compatibility_context = nullptr;
	}
	auto window_pointer = window.get();
	update_event_listeners.emplace_back(owning_loop.on_begin_update.listen([this, window_pointer] {
		window_pointer->poll();
	}));
	draw_event_listeners.emplace_back(window->on_draw_begin.listen([this, window_pointer] {
		context->make_current(*window_pointer);
		context->clear();
	}));
	return window;
}

std::shared_ptr<render_context> window_manager::get_render_context() const {
	return context;
}

imgui_instance::imgui_instance(loop& loop, window& window, render_context& context_) : context{ context_ } {
	ui::create(window);
	ui::add_font("calibril.ttf", 16);
	ui::add_font("calibril.ttf", 12);
	ui::build_fonts();
	draw_event_listener = window.on_draw.listen([this] {
		ui::draw(context);
	});
	begin_update_event_listener = loop.on_begin_update.listen([this] {
		ui::start_frame();
	});
	end_update_event_listener = loop.on_end_update.listen([this] {
		debug::menu::update();
		ui::end_frame();
	});
}

imgui_instance::~imgui_instance() {
	ui::destroy();
}

audio_component::audio_component() {
	endpoint = std::make_unique<wasapi::audio_device>();
}

}
