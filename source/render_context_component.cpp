#include "imgui_loop_component.hpp"
#include "graphics/gl/wgl_context.hpp"
#include "graphics/windows_window.hpp"
#include "audio/wasapi.hpp"
#include "network/winsock_socket.hpp"
#include "network/winsock_connection_manager.hpp"
#include "loop.hpp"
#include "editor.hpp"

namespace nfwk {

window_manager::window_manager(loop& loop) : owning_loop{ loop } {
	platform::windows_window::create_classes();
	frame_event_listener = owning_loop.on_begin_frame.listen([this] {
		for (const auto& window : windows) {
			window->on_draw_begin.emit();
			window->on_draw.emit();
			window->on_draw_end.emit();
		}
	});
}

std::shared_ptr<window> window_manager::create_window(std::string_view title, std::optional<vector2i> size) {
	auto compatibility_context = context ? nullptr : platform::windows_window::create_compatibility_render_context();
	auto& window = windows.emplace_back(std::make_shared<platform::windows_window>(title, size));
	if (!context) {
		context = window->create_render_context(std::nullopt);
		context->set_clear_color(0.1f);
		context->log_info();
		compatibility_context = nullptr;
	}
	auto window_pointer = window.get();
	update_event_listeners.emplace_back(owning_loop.on_begin_update.listen([this, window_pointer] {
		window_pointer->poll();
	}, make_priority(4, 0)));
	draw_event_listeners.emplace_back(window->on_draw_begin.listen([this, window_pointer] {
		context->make_current(*window_pointer);
		context->clear();
	}));
	return window;
}

std::shared_ptr<render_context> window_manager::get_render_context() const {
	return context;
}

std::shared_ptr<ui_manager> window_manager::get_ui_manager() {
	if (!ui && !windows.empty()) {
		ui = std::make_shared<ui_manager>(owning_loop, *windows[0], *context);
	}
	return ui;
}

ui_manager::ui_manager(loop& loop, window& window, render_context& context_) : context{ context_ } {
	info(ui::log, "Constructing UI manager");
	menu_bar = std::make_shared<ui::main_menu_bar>(loop);
	log_ui = std::make_unique<ui::integrated_log_ui>(menu_bar);
	windows = std::make_unique<ui::window_container>();
	ui::create(window);
	ui::add_font("calibril.ttf", 16);
	ui::add_font("calibril.ttf", 12);
	ui::build_fonts();
	draw_event_listener = window.on_draw.listen([this] {
		ui::draw(context);
	});
	begin_update_event_listener = loop.on_begin_update.listen([this] {
		ui::start_frame();
		windows->update();
	}, priority::lowest);
	end_update_event_listener = loop.on_end_update.listen([this] {
		menu_bar->update();
		ui::end_frame();
	});
}

ui_manager::~ui_manager() {
	info(ui::log, "Destructing UI manager");
	ui::destroy();
}

std::shared_ptr<ui::main_menu_bar> ui_manager::get_main_menu_bar() const {
	return menu_bar;
}

ui::window_container& ui_manager::get_window_container() {
	return *windows;
}

audio_component::audio_component() {
	endpoint = std::make_unique<wasapi::audio_device>();
}

network_manager::network_manager() {
	manager = std::make_unique<winsock_connection_manager>();
}

network_socket& network_manager::make_listener(std::string_view address, int port) {
	auto socket = manager->create_socket();
	socket->load_address(address, port);
	socket->bind_and_listen();
	return *socket;
}

connection_manager& network_manager::connections() {
	return *manager;
}

std::string system_command_manager::user_agent;

system_command_manager::system_command_manager(event<>& update_event) {
	update_event_listener = update_event.listen([this] {
		for (int i{ 0 }; i < static_cast<int>(runners.size()); i++) {
			if (runners[i] && runners[i]->finish()) {
				runners.erase(runners.begin() + i);
				i--;
			}
		}
	});
}

int system_command_manager::active_commands() const {
	return static_cast<int>(runners.size());
}

void system_command_manager::run(const std::string& command, const std::function<void(io_stream*)>& on_done) {
	if (!command.empty()) {
		runners.emplace_back(std::make_unique<platform::system_command_runner>(command, on_done));
	}
}

void system_command_manager::http_request(std::string_view path, std::string_view method, std::string_view data, std::string_view type, const std::function<void(io_stream*)>& on_done, std::string_view cookies_path) {
	run(get_curl_command(path, method, data, type, cookies_path), on_done);
}

void system_command_manager::http_get(std::string_view path, const std::function<void(io_stream*)>& on_done, std::string_view cookies_path) {
	http_request(path, "GET", "", "", on_done, cookies_path);
}

io_stream system_command_manager::run(const std::string& command) {
	if (command.empty()) {
		return {};
	}
	platform::system_command_runner runner{ command, {} };
	while (!runner.is_done()) {
		platform::sleep(1);
	}
	auto stream = runner.get_stream();
	return { stream->at_read(), stream->size_left_to_read(), io_stream::const_construct_by::copy };
}

io_stream system_command_manager::http_request(std::string_view path, std::string_view method, std::string_view data, std::string_view type, std::string_view cookies_path) {
	return run(get_curl_command(path, method, data, type, cookies_path));
}

io_stream system_command_manager::http_get(std::string_view path, std::string_view cookies_path) {
	return http_request(path, "GET", "", "", cookies_path);
}

void system_command_manager::set_user_agent(const std::string& new_user_agent) {
	user_agent = new_user_agent;
}

std::string system_command_manager::get_curl_command(std::string_view path, std::string_view method, std::string_view data, std::string_view type, std::string_view cookies_path) {
	if (path.find('"') != std::string_view::npos) {
		error("core", "Can't download %cyan{}%red. Contains invalid character.", path);
		return {};
	}
	if (user_agent.empty()) {
		// some websites just don't like programmers with their cool "curl" user agent, I guess.
		set_user_agent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/90.0.4430.212 Safari/537.36");
	}
	std::string base;
	if (cookies_path.empty()) {
		base = std::format("curl --silent --user-agent \"{}\"", user_agent);
	} else {
		base = std::format("curl --silent --user-agent \"{}\" --cookies \"{}\"", user_agent, cookies_path);
	}
	if (data.empty()) {
		return std::format("{} --request {} \"{}\"", base, method, path);
	}
	if (type.empty()) {
		type = "application/x-www-form-urlencoded";
	}
	return std::format("{} -d '{}' -H {} --request {} \"{}\"", base, data, type, method, path);
}

}
