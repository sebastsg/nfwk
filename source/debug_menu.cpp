#include "debug_menu.hpp"
#include "html_log_writer.hpp"
#include "loop.hpp"
#include "graphics/ui.hpp"
#include "graphics/draw.hpp"

namespace nfwk::debug::menu {

struct menu_info {
	std::u8string id;
	std::function<void()> update;
};

static struct {
	bool enabled{ false };
	bool initialized{ false };
	bool limit_fps{ true };
	bool show_fps{ true };
	std::unordered_map<std::u8string, std::vector<menu_info>> menus;
	loop* owning_loop{ nullptr };
	bool imgui_demo_enabled{ false };
	std::unordered_set<std::u8string> open_log_windows;
} menu_state;

void add(std::u8string_view id, std::u8string_view name, const std::function<void()>& update) {
	auto& menu = menu_state.menus[name.data()].emplace_back();
	menu.id = id;
	menu.update = update;
}

void add(std::u8string_view id, const std::function<void()>& update) {
	add(id, u8"", update);
}

void enable(loop& owning_loop) {
	menu_state.enabled = true;
	menu_state.owning_loop = &owning_loop;
	if (!menu_state.initialized) {
		add(u8"nfwk-debug", u8"Options", [] {
			// todo: need to have a way to swap loops to add this functionality back.
			//if (ui::menu_item("Limit FPS", menu_state.limit_fps)) {}
			ui::menu_item(u8"Show FPS", menu_state.show_fps);
			if (auto _ = ui::menu(u8"Debug logs")) {
				for (auto& log : log::get_logs()) {
					if (auto log_menu = ui::menu(log->name)) {
						ui::menu_item(u8"Show time", log->show_time);
						ui::menu_item(u8"Show file", log->show_file);
						ui::menu_item(u8"Show line", log->show_line);
						if (ui::menu_item(u8"Open in browser")) {
							bool absent{ true };
							for (auto& writer : log->get_writers()) {
								if (typeid(*writer) == typeid(log::html_writer)) {
									writer->open();
									absent = false;
									break;
								}
							}
							if (absent) {
								auto writer = std::make_unique<log::html_writer>(log);
								writer->flush();
								writer->open();
								log::add_writer(std::move(writer));
							}
						}
						bool is_window_open{ menu_state.open_log_windows.find(log->name) != menu_state.open_log_windows.end() };
						if (ui::menu_item(u8"Integrated view", is_window_open)) {
							if (is_window_open) {
								menu_state.open_log_windows.insert(log->name);
							} else {
								menu_state.open_log_windows.erase(log->name);
							}
						}
						ui::separate();
						for (const auto& entry : log->get_entries()) {
							ui::menu_item(u8"[" + entry.time + u8"] " + entry.file + u8":" + to_string(entry.line) + u8": " + entry.message);
						}
					}
				}
			}
		});
		add(u8"nfwk-debug", u8"View", [] {
			ui::menu_item(u8"ImGui Demo", menu_state.imgui_demo_enabled);
		});
		menu_state.initialized = true;
	}
}

void disable() {
	menu_state.enabled = false;
}

void update() {
	if (!menu_state.enabled) {
		return;
	}
	ImGui::BeginMainMenuBar();
	for (const auto& [name, menus] : menu_state.menus) {
		if (!menus.empty() && !name.empty()) {
			if (auto _ = ui::menu(name)) {
				for (auto& menu : menus) {
					menu.update();
				}
			}
		}
	}
	for (const auto& [name, menus] : menu_state.menus) {
		if (!menus.empty() && name.empty()) {
			for (auto& menu : menus) {
				menu.update();
			}
		}
	}
	if (menu_state.show_fps) {
		ui::colored_text({ 0.9f, 0.9f, 0.1f }, u8"\tFPS: %i", menu_state.owning_loop->current_fps());
	}
	ImGui::EndMainMenuBar();
	constexpr vector3f log_color[]{
		{ 1.0f, 1.0f, 1.0f }, // message
		{ 1.0f, 0.9f, 0.2f }, // warning
		{ 1.0f, 0.4f, 0.4f }, // error
		{ 0.4f, 0.8f, 1.0f }  // info
	};
	for (const auto& name : menu_state.open_log_windows) {
		bool open{ true };
		if (auto _ = ui::window(name, std::nullopt, std::nullopt, ImGuiWindowFlags_AlwaysAutoResize, &open)) {
			if (!open) {
				menu_state.open_log_windows.erase(name);
				break;
			}
			if (auto log = log::find_log(name)) {
				for (const auto& entry : log->get_entries()) {
					if (log->show_time) {
						ui::colored_text({ 0.4f, 0.35f, 0.3f }, u8"%s", entry.time.c_str());
						ui::inline_next();
					}
					if (log->show_file) {
						ui::colored_text({ 0.5f, 0.6f, 0.7f }, u8"%s", entry.file.c_str());
						ui::inline_next();
					}
					if (log->show_line) {
						ui::colored_text({ 0.3f, 0.3f, 0.3f }, u8"%i", entry.line);
						ui::inline_next();
					}
					ui::colored_text(log_color[static_cast<int>(entry.type)], u8"%s", entry.message.c_str());
				}
			}
		}
	}
	if (menu_state.imgui_demo_enabled) {
		ImGui::ShowDemoWindow();
	}
}

void remove(std::u8string_view id) {
	for (auto& [name, menus] : menu_state.menus) {
		for (int i{ 0 }; i < static_cast<int>(menus.size()); i++) {
			if (menus[i].id == id) {
				menus.erase(menus.begin() + i);
				i--;
			}
		}
	}
}

}
