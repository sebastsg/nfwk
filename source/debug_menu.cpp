#include "debug_menu.hpp"
#include "html_log_writer.hpp"
#include "loop.hpp"
#include "frame_rate_controller.hpp"

namespace nfwk::debug::menu {

struct menu_info {
	std::string id;
	std::function<void()> update;
};

static struct {
	bool enabled{ false };
	bool initialized{ false };
	std::unordered_map<std::string, std::vector<menu_info>> menus;
} menu_state;

static std::unordered_set<std::string> open_log_windows;
static bool imgui_demo_enabled{ false };

void add(std::string_view id, std::string_view name, const std::function<void()>& update) {
	auto& menu = menu_state.menus[name.data()].emplace_back();
	menu.id = id;
	menu.update = update;
}

void add(std::string_view id, const std::function<void()>& update) {
	add(id, "", update);
}

void enable(loop& loop) {
	menu_state.enabled = true;
	if (!menu_state.initialized) {
		add("nfwk-debug", "Options", [] {
			bool limit_fps{ true };
			if (ui::menu_item("Limit FPS", limit_fps)) {
				//
			}
			if (auto _ = ui::menu("Debug logs")) {
				for (auto& log : log::get_logs()) {
					if (auto log_menu = ui::menu(log->name)) {
						ui::menu_item("Show time", log->show_time);
						ui::menu_item("Show file", log->show_file);
						ui::menu_item("Show line", log->show_line);
						if (ui::menu_item("Open in browser")) {
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
						bool is_window_open{ open_log_windows.find(log->name) != open_log_windows.end() };
						if (ui::menu_item("Integrated view", is_window_open)) {
							if (is_window_open) {
								open_log_windows.insert(log->name);
							} else {
								open_log_windows.erase(log->name);
							}
						}
						ui::separate();
						for (const auto& entry : log->get_entries()) {
							ui::menu_item("[" + entry.time + "] " + entry.file + ":" + std::to_string(entry.line) + ": " + entry.message);
						}
					}
				}
			}
		});
		add("nfwk-debug", "View", [] {
			ui::menu_item("ImGui Demo", imgui_demo_enabled);
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
	///ui::colored_text({ 0.9f, 0.9f, 0.1f }, "\tFPS: %i", frame_counter().current_fps());
	ImGui::EndMainMenuBar();
	constexpr vector3f log_color[]{
		{ 1.0f, 1.0f, 1.0f }, // message
		{ 1.0f, 0.9f, 0.2f }, // warning
		{ 1.0f, 0.4f, 0.4f }, // error
		{ 0.4f, 0.8f, 1.0f }  // info
	};
	for (const auto& name : open_log_windows) {
		bool open{ true };
		if (auto _ = ui::push_window(name, std::nullopt, std::nullopt, ImGuiWindowFlags_AlwaysAutoResize, &open)) {
			if (!open) {
				open_log_windows.erase(name);
				break;
			}
			if (auto log = log::find_log(name)) {
				for (const auto& entry : log->get_entries()) {
					if (log->show_time) {
						ui::colored_text({ 0.4f, 0.35f, 0.3f }, "%s", entry.time.c_str());
						ui::inline_next();
					}
					if (log->show_file) {
						ui::colored_text({ 0.5f, 0.6f, 0.7f }, "%s", entry.file.c_str());
						ui::inline_next();
					}
					if (log->show_line) {
						ui::colored_text({ 0.3f, 0.3f, 0.3f }, "%i", entry.line);
						ui::inline_next();
					}
					ui::colored_text(log_color[static_cast<int>(entry.type)], "%s", entry.message.c_str());
				}
			}
		}
	}
	if (imgui_demo_enabled) {
		ImGui::ShowDemoWindow();
	}
}

void remove(std::string_view id) {
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
