module;

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

export module nfwk.ui:debug_menu;

import std.core;
import nfwk.core;
import :imgui_wrapper;

struct menu_info {
	std::string id;
	std::function<void()> update;
};

static struct {
	bool enabled{ false };
	bool initialized{ false };
	std::unordered_map<std::string, std::vector<menu_info>> menus;
} menu_state;

std::unordered_set<std::string> open_log_windows;
bool imgui_demo_enabled{ false };

export namespace nfwk::debug::menu {

void add(std::string_view id, std::string_view name, const std::function<void()>& update) {
	auto& menu = menu_state.menus[name.data()].emplace_back();
	menu.id = id;
	menu.update = update;
}

void add(std::string_view id, const std::function<void()>& update) {
	add(id, "", update);
}

void enable() {
	menu_state.enabled = true;
	if (!menu_state.initialized) {
		add("nfwk-debug", "Options", [] {
#if 0
			bool limit_fps{ get_draw_synchronization() == draw_synchronization::if_updated };
			if (ui::menu_item("Limit FPS", limit_fps)) {
				set_draw_synchronization(limit_fps ? nfwk::draw_synchronization::if_updated : nfwk::draw_synchronization::always);
			}
			if (auto end = ui::menu("Debug logs")) {
				ui::menu_item("Show time", log::log_flags.show_time);
				ui::menu_item("Show file", log::log_flags.show_file);
				ui::menu_item("Show line", log::log_flags.show_line);
				ui::separate();
				for (const auto& [name, log] : log::internal::logs) {
					if (auto log_menu = ui::menu(name)) {
						if (ui::menu_item("Open in browser")) {
							const bool absent = std::none_of(log::internal::writers.begin(), log::internal::writers.end(), [&name](const auto& writer) {
								return writer->name == name;
							});
							if (absent) {
								log::add_writer<log::html_writer>(name);
							}
							for (const auto& writer : log::internal::writers) {
								if (writer->name == name) {
									writer->open();
									break;
								}
							}
						}
						bool is_window_open{ open_log_windows.contains(name) };
						if (ui::menu_item("Integrated view", is_window_open)) {
							if (is_window_open) {
								open_log_windows.insert(name);
							} else {
								open_log_windows.erase(name);
							}
						}
						ui::separate();
						for (const auto& entry : log.entries()) {
							ui::menu_item("[" + std::to_string(entry.time) + "] " + entry.file + ":" + std::to_string(entry.line) + ": " + entry.message);
						}
					}
				}
			}
#endif
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
		if (menus.empty()) {
			continue;
		}
		if (name.empty()) {
			for (auto& menu : menus) {
				menu.update();
			}
		} else {
			if (auto end = ui::menu(name)) {
				for (auto& menu : menus) {
					menu.update();
				}
			}
		}
	}
	///ui::colored_text({ 0.9f, 0.9f, 0.1f }, "\tFPS: %i", frame_counter().current_fps());
	ImGui::EndMainMenuBar();
	constexpr vector3f log_color[]{
		{ 1.0f, 1.0f, 1.0f }, // message
		{ 1.0f, 0.9f, 0.2f }, // warning
		{ 1.0f, 0.4f, 0.4f }, // critical
		{ 0.4f, 0.8f, 1.0f }  // info
	};
	for (const auto& name : open_log_windows) {
		bool open{ true };
		if (auto end = ui::window(name, ImGuiWindowFlags_AlwaysAutoResize, &open)) {
			if (!open) {
				open_log_windows.erase(name);
				break;
			}
			auto& log = log::find_log(name)->get();
			for (const auto& entry : log.get_entries()) {
				if (log.show_time) {
					ui::colored_text({ 0.4f, 0.35f, 0.3f }, "%s", entry.time.c_str());
					ui::inline_next();
				}
				if (log.show_file) {
					ui::colored_text({ 0.5f, 0.6f, 0.7f }, "%s", entry.file.c_str());
					ui::inline_next();
				}
				if (log.show_line) {
					ui::colored_text({ 0.3f, 0.3f, 0.3f }, "%i", entry.line);
					ui::inline_next();
				}
				ui::colored_text(log_color[static_cast<int>(entry.type)], "%s", entry.message.c_str());
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
