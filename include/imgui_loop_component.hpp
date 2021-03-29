#pragma once

#include "subprogram.hpp"
#include "imgui_platform.hpp"
#include "graphics/window.hpp"
#include "network/network.hpp"
#include "audio/audio_endpoint.hpp"

namespace nfwk {

class imgui_instance {
public:

	imgui_instance(loop& loop, window& window, render_context& context);
	imgui_instance(const imgui_instance&) = delete;
	imgui_instance(imgui_instance&&) = delete;
	
	~imgui_instance();

	imgui_instance& operator=(const imgui_instance&) = delete;
	imgui_instance& operator=(imgui_instance&&) = delete;

private:

	event_listener draw_event_listener;
	event_listener begin_update_event_listener;
	event_listener end_update_event_listener;
	render_context& context;

};

class window_manager {
public:

	window_manager(loop& loop, bool support_imgui);

	std::shared_ptr<window> create_window(std::u8string_view title, std::optional<vector2i> size = std::nullopt);
	std::shared_ptr<render_context> get_render_context() const;

private:

	std::vector<std::shared_ptr<window>> windows;
	std::shared_ptr<render_context> context;
	std::vector<event_listener> draw_event_listeners;
	std::vector<event_listener> update_event_listeners;
	event_listener frame_event_listener;
	loop& owning_loop;
	std::unique_ptr<imgui_instance> imgui;
	bool support_imgui{ false };

};

class network_component {
public:

	network_component() {
		start_network();
	}

	~network_component() {
		stop_network();
	}

};

class audio_component {
public:

	audio_component();

private:

	std::unique_ptr<audio_endpoint> endpoint;

};

}
