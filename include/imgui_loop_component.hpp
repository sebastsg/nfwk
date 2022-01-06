#pragma once

#include "subprogram.hpp"
#include "imgui_platform.hpp"
#include "graphics/window.hpp"
#include "network/network.hpp"
#include "audio/audio_endpoint.hpp"
#include "debug_menu.hpp"

namespace nfwk::ui {
class window_container;
}

namespace nfwk {

class ui_manager {
public:

	ui_manager(loop& loop, window& window, render_context& context);
	ui_manager(const ui_manager&) = delete;
	ui_manager(ui_manager&&) = delete;
	
	~ui_manager();

	ui_manager& operator=(const ui_manager&) = delete;
	ui_manager& operator=(ui_manager&&) = delete;

	std::shared_ptr<ui::main_menu_bar> get_main_menu_bar() const;
	ui::window_container& get_window_container();

private:

	event_listener draw_event_listener;
	event_listener begin_update_event_listener;
	event_listener end_update_event_listener;
	render_context& context;
	std::shared_ptr<ui::main_menu_bar> menu_bar;
	std::unique_ptr<ui::integrated_log_ui> log_ui;
	std::unique_ptr<ui::window_container> windows;

};

class window_manager {
public:

	window_manager(loop& loop);

	std::shared_ptr<window> create_window(std::string_view title, std::optional<vector2i> size = std::nullopt);
	std::shared_ptr<render_context> get_render_context() const;
	std::shared_ptr<ui_manager> get_ui_manager();

private:

	std::vector<std::shared_ptr<window>> windows;
	std::shared_ptr<render_context> context;
	std::vector<event_listener> draw_event_listeners;
	std::vector<event_listener> update_event_listeners;
	event_listener frame_event_listener;
	loop& owning_loop;
	std::shared_ptr<ui_manager> ui;

};

class audio_component {
public:

	audio_component();

private:

	std::unique_ptr<audio_endpoint> endpoint;

};

class network_manager {
public:

	network_manager();
	network_manager(const network_manager&) = delete;
	network_manager(network_manager&&) = delete;

	~network_manager() = default;

	network_manager& operator=(const network_manager&) = delete;
	network_manager& operator=(network_manager&&) = delete;

	network_socket& make_listener(std::string_view address, int port);

	connection_manager& connections();

private:

	std::unique_ptr<connection_manager> manager;
	
};

class system_command_manager {
public:

	system_command_manager(event<>& update_event);
	system_command_manager(const system_command_manager&) = delete;
	system_command_manager(system_command_manager&&) = delete;

	~system_command_manager() = default;

	system_command_manager& operator=(const system_command_manager&) = delete;
	system_command_manager& operator=(system_command_manager&&) = delete;

	int active_commands() const;
	
	void run(const std::string& command, const std::function<void(io_stream*)>& on_done);

	// todo: move these to 'http' module when it's made.
	
	void http_request(std::string_view path, std::string_view method, std::string_view data, std::string_view type, const std::function<void(io_stream*)>& on_done, std::string_view cookies_path = "");
	void http_get(std::string_view path, const std::function<void(io_stream*)>& on_done, std::string_view cookies_path = "");

	// blocking
	static io_stream run(const std::string& command);
	static io_stream http_request(std::string_view path, std::string_view method, std::string_view data, std::string_view type, std::string_view cookies_path = "");
	static io_stream http_get(std::string_view path, std::string_view cookies_path = "");

	static void set_user_agent(const std::string& user_agent);

private:

	static std::string get_curl_command(std::string_view path, std::string_view method, std::string_view data, std::string_view type, std::string_view cookies_path);

	event_listener update_event_listener;
	std::vector<std::unique_ptr<platform::system_command_runner>> runners;

	static std::string user_agent;

};

}
