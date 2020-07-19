#include "debug.hpp"
#include "assets.hpp"
#include "loop.hpp"
#include "ui.hpp"

#include <chrono>
#include <iostream>
#include <iomanip>

namespace no::debug::internal {

// this can be used for release builds to have fewer external files.
static constexpr std::string_view default_template_html{
	"<!doctype html><html><head><style>"
	"body { margin: 0; background: #202021; }"
	"table { width: 100%; border-collapse: collapse; }"
	"td { border: 1px solid #333; background: #282828; color: #a0a0a0; font-family: monospace; }"
	".warning td { border-left: 2px solid #ff0; border-right: 2px solid #ff0; }"
	".critical td { border-left: 2px solid #f00; border-right: 2px solid #f00; }"
	".info td { border-left: 2px solid #14c8c8e6; border-right: 2px solid #14c8c8e6; }"
	"</head></style><body><table><tr class\"colhead\"><td>Time</td><td>Message</td>"
	"<td>File</td><td style=\"width:25%;\">Function</td><td>Line</td></tr>"
};

static std::string template_html{ default_template_html };

void initialize_debug() {
	if (auto buffer = file::read(asset_path("debug/template.html")); !buffer.empty()) {
		template_html = buffer;
	}
}

}

namespace no::debug {

static auto current_time_ms() {
	const auto time_since_epoch = std::chrono::system_clock::now().time_since_epoch();
	const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch).count();
	return milliseconds % 1000;
}

static std::string current_time_string_for_log() {
	return platform::current_local_time_string() + "." + std::to_string(current_time_ms());
}

static void replace_substring(std::string& string, std::string_view substring, std::string_view replace_with) {
	auto index = string.find(substring);
	while (index != std::string::npos) {
		string.replace(index, substring.size(), replace_with);
		index = string.find(substring, index + replace_with.size());
	}
}

static std::string get_html_compatible_string(std::string string) {
	replace_substring(string, "&", "&amp;");
	replace_substring(string, ">", "&gt;");
	replace_substring(string, "<", "&lt;");
	replace_substring(string, "\n", "<br>");
	replace_substring(string, "[b]", "<b>");
	replace_substring(string, "[/b]", "</b>");
	return string;
}

debug_log_entry::debug_log_entry(message_type type, std::string_view message, std::string_view path, std::string_view function, int line)
	: type{ type }, message{ message }, function{ function }, line{ line }, time{ current_time_string_for_log() } {
	if (auto slash = path.rfind(std::filesystem::path::preferred_separator); slash != std::string::npos) {
		file = path.substr(slash + 1);
	} else {
		file = path;
	}
}

debug_log::debug_log(const std::string& name) : log_name{ name } {

}

std::string_view debug_log::name() const {
	return log_name;
}

int debug_log::count() const {
	return static_cast<int>(log_entries.size());
}

const std::vector<debug_log_entry>& debug_log::entries() const {
	return log_entries;
}

std::string html_debug_log_writer::entry_html(const debug_log_entry& entry) {
	std::string html{ "\r\n<tr class=\"" + STRING(entry.type) + "\">" };
	html += field_html(entry.time);
	html += field_html(get_html_compatible_string(entry.message));
	html += field_html(entry.file);
	html += field_html(get_html_compatible_string(entry.function));
	html += field_html(std::to_string(entry.line));
	html += "</tr>";
	return html;
}

std::string html_debug_log_writer::field_html(const std::string& message, int col_span) {
	return "<td colspan=\"" + std::to_string(col_span) + "\">" + message + "</td>";
}

class logger_state {
	void add(int index, message_type type, const char* file, const char* func, int line) {
		//file::append(html_logs[index].path, html_logs[index].final_buffer);
	}
	void initialize_html_log(int index) {
		//html_logs[index].path = "debug-log-" + std::to_string(index) + ".html";
		//file::write(html_logs[index].path, template_buffer);
	}
};

static std::unordered_map<std::string, debug_log> logs;

void append(const std::string& name, message_type type, std::string_view file, std::string_view function, int line, std::string_view message) {
	const auto& [log, _] = logs.try_emplace(name, name);
	const auto& entry = log->second.add(type, message, file, function, line);

#if ENABLE_STDOUT_LOG
	std::cout << std::left << std::setw(13) << (current_local_time_string() + "." + current_time_ms_string()) << std::setw(1);
	std::cout << std::internal << type << ": " << message << "\n";
#endif
}

}

namespace no::debug::menu {

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
static struct {
	bool show_time{ true };
	bool show_file{ true };
	bool show_line{ true };
} log_window_flags;

void enable() {
	menu_state.enabled = true;
	if (!menu_state.initialized) {
		add("nfwk-debug", "Options", [] {
			bool limit_fps{ get_draw_synchronization() == draw_synchronization::if_updated };
			if (ui::menu_item("Limit FPS", limit_fps)) {
				set_draw_synchronization(limit_fps ? no::draw_synchronization::if_updated : no::draw_synchronization::always);
			}
			if (auto end = ui::menu("Debug logs")) {
				ui::menu_item("Show time", log_window_flags.show_time);
				ui::menu_item("Show file", log_window_flags.show_file);
				ui::menu_item("Show line", log_window_flags.show_line);
				ui::separate();
				for (const auto& [name, log] : logs) {
					if (auto log_menu = ui::menu(name)) {
						if (ui::menu_item("Open in browser")) {
							platform::open_file(std::string{ log.name() } + ".html", false);
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
							ui::menu_item(STRING("[" << entry.time << "] " << entry.file << ":" << entry.line << ": " << entry.message));
						}
					}
				}
			}
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
	ImGui::EndMainMenuBar();
	for (const auto& log_name : open_log_windows) {
		const auto& log = logs.find(log_name)->second;
		bool open{ true };
		if (auto end = ui::push_window(log_name, std::nullopt, std::nullopt, ImGuiWindowFlags_AlwaysAutoResize, &open)) {
			if (!open) {
				open_log_windows.erase(log_name);
				break;
			}
			for (const auto& entry : log.entries()) {
				std::string text;
				if (log_window_flags.show_time) {
					ui::colored_text({ 0.4f, 0.35f, 0.3f }, "%s", entry.time.c_str());
					ui::inline_next();
				}
				if (log_window_flags.show_file) {
					ui::colored_text({ 0.5f, 0.6f, 0.7f }, "%s", entry.file.c_str());
					ui::inline_next();
				}
				if (log_window_flags.show_line) {
					ui::colored_text({ 0.3f, 0.3f, 0.3f }, "%i", entry.line);
					ui::inline_next();
				}
				ui::colored_text({ 1.0f, 1.0f, 1.0f }, "%s", entry.message.c_str());
			}
		}
	}
}

void add(std::string_view id, std::string_view name, const std::function<void()>& update) {
	auto& menu = menu_state.menus[name.data()].emplace_back();
	menu.id = id;
	menu.update = update;
}

void add(std::string_view id, const std::function<void()>& update) {
	add(id, "", update);
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

std::ostream& operator<<(std::ostream& out, no::debug::message_type message) {
	switch (message) {
	case no::debug::message_type::message: return out << "message";
	case no::debug::message_type::warning: return out << "warning";
	case no::debug::message_type::critical: return out << "critical";
	case no::debug::message_type::info: return out << "info";
	default: return out << "";
	}
}
