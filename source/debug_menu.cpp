#include "debug_menu.hpp"
#include "loop.hpp"
#include "html_log_writer.hpp"
#include "graphics/ui.hpp"

namespace nfwk::ui {

main_menu_bar::main_menu_bar(loop& loop) : owning_loop{ loop } {
	add("nfwk-fps-options", "Options", [this] {
		checkable_menu_item("Limit FPS", limit_fps); // todo: need a way to swap loops to add this functionality back.
		checkable_menu_item("Show FPS", show_fps);
	});
	add("nfwk-ui-demo", "View", [this] {
		checkable_menu_item("ImGui Demo", show_imgui_demo);
	});
}

void main_menu_bar::add(std::string_view id, std::string_view name, std::function<void()> update) {
	auto& entry_group = user_entry_groups[name.data()].emplace_back();
	entry_group.id = id;
	entry_group.update = std::move(update);
}

void main_menu_bar::add(std::string_view id, std::function<void()> update) {
	add(id, "", std::move(update));
}

void main_menu_bar::remove(std::string_view id) {
	for (auto& [_, user_entry_group] : user_entry_groups) {
		// todo: c++20 erase_if
		for (int i{ 0 }; i < static_cast<int>(user_entry_group.size()); i++) {
			if (user_entry_group[i].id == id) {
				user_entry_group.erase(user_entry_group.begin() + i);
				i--;
			}
		}
	}
}

void main_menu_bar::update() {
	if (!visible) {
		return;
	}
	ImGui::BeginMainMenuBar();
	for (const auto& [name, user_menu_entry_group] : user_entry_groups) {
		if (!user_menu_entry_group.empty() && !name.empty()) {
			if (auto _ = menu(name)) {
				for (auto& user_menu_entry : user_menu_entry_group) {
					user_menu_entry.update();
				}
			}
		}
	}
	for (const auto& [name, user_menu_entry_group] : user_entry_groups) {
		if (!user_menu_entry_group.empty() && name.empty()) {
			for (auto& user_menu_entry : user_menu_entry_group) {
				user_menu_entry.update();
			}
		}
	}
	if (show_fps) {
		colored_text({ 0.9f, 0.9f, 0.1f }, "\tFPS: %i", owning_loop.current_fps());
	}
	ImGui::EndMainMenuBar();

	// here: update integrated log ui
	
	if (show_imgui_demo) {
		ImGui::ShowDemoWindow();
	}
}

integrated_log_ui::integrated_log_ui(std::shared_ptr<main_menu_bar> menu_bar_) : menu_bar{ std::move(menu_bar_) } {
	menu_bar->add("nfwk-logs", "Options", [this] {
		if (auto _ = menu("Debug logs")) {
			for (auto& log : log::get_logs()) {
				if (auto log_menu = menu(log->name)) {
					checkable_menu_item("Show time", log->show_time);
					checkable_menu_item("Show file", log->show_file);
					checkable_menu_item("Show line", log->show_line);
					if (menu_item("Open in browser")) {
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
					if (checkable_menu_item("Integrated view", is_window_open)) {
						if (is_window_open) {
							open_log_windows.insert(log->name);
						} else {
							open_log_windows.erase(log->name);
						}
					}
					separate();
					for (const auto& entry : log->get_entries()) {
						menu_item("[" + entry.time + "] " + entry.file + ":" + std::to_string(entry.line) + ": " + entry.message);
					}
				}
			}
		}
	});
}

integrated_log_ui::~integrated_log_ui() {
	menu_bar->remove("nfwk-logs");
}

void integrated_log_ui::update() {
	constexpr vector3f log_color[]{
		{ 1.0f, 1.0f, 1.0f }, // message
		{ 1.0f, 0.9f, 0.2f }, // warning
		{ 1.0f, 0.4f, 0.4f }, // error
		{ 0.4f, 0.8f, 1.0f }  // info
	};
	for (const auto& name : open_log_windows) {
		bool open{ true };
		if (auto _ = window(name, std::nullopt, std::nullopt, ImGuiWindowFlags_AlwaysAutoResize, &open)) {
			if (!open) {
				open_log_windows.erase(name);
				break;
			}
			if (auto log = log::find_log(name)) {
				for (const auto& entry : log->get_entries()) {
					if (log->show_time) {
						colored_text({ 0.4f, 0.35f, 0.3f }, "%s", entry.time.c_str());
						inline_next();
					}
					if (log->show_file) {
						colored_text({ 0.5f, 0.6f, 0.7f }, "%s", entry.file.c_str());
						inline_next();
					}
					if (log->show_line) {
						colored_text({ 0.3f, 0.3f, 0.3f }, "%i", entry.line);
						inline_next();
					}
					colored_text(log_color[static_cast<int>(entry.type)], "%s", entry.message.c_str());
				}
			}
		}
	}
}

}
