#include "debug.hpp"

#if ENABLE_DEBUG

#include "io.hpp"
#include "assets.hpp"
#include "loop.hpp"

#include <chrono>
#include <mutex>
#include <iostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& out, no::debug::message_type message) {
	switch (message) {
	case no::debug::message_type::message: return out << "message";
	case no::debug::message_type::warning: return out << "warning";
	case no::debug::message_type::critical: return out << "critical";
	case no::debug::message_type::info: return out << "info";
	default: return out << "";
	}
}

namespace no::debug {

static std::string current_time_ms_string() {
	const auto time_since_epoch = std::chrono::system_clock::now().time_since_epoch();
	const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch).count();
	return std::to_string(milliseconds % 1000);
}

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

class logger_state {
public:

	std::mutex mutex;

#if ENABLE_HTML_LOG

	struct html_log {
		std::string path;
		std::string temp_buffer;
		std::string final_buffer;
	};

	void add(int index, message_type type, const char* file, const char* func, int line) {
		if (template_buffer.empty()) {
			template_buffer = file::read(asset_path("debug/template.html"));
			if (template_buffer.empty()) {
				template_buffer = default_template_html;
			}
			for (const auto& log : html_logs) {
				file::write(log.path, template_buffer);
			}
		}
		if (html_logs[index].temp_buffer.empty()) {
			return;
		}
		html_logs[index].final_buffer += "\r\n<tr class=\"" + STRING(type) + "\">";
		write_field(index, "<b>" + current_local_time_string() + "</b>." + current_time_ms_string());
		write_field(index, html_logs[index].temp_buffer);
		write_field(index, file);
		write_field(index, get_html_compatible_string(func));
		write_field(index, std::to_string(line));
		html_logs[index].final_buffer += "</tr>";
		html_logs[index].temp_buffer = "";
		if (template_buffer.empty()) {
			return;
		}
		file::append(html_logs[index].path, html_logs[index].final_buffer);
		html_logs[index].final_buffer = "";
	}

	std::string get_html_compatible_string(std::string string) {
		replace_substring(string, "&", "&amp;");
		replace_substring(string, ">", "&gt;");
		replace_substring(string, "<", "&lt;");
		replace_substring(string, "\n", "<br>");
		replace_substring(string, "[b]", "<b>");
		replace_substring(string, "[/b]", "</b>");
		return string;
	}

	html_log* get_html_log(int index) {
		if (index < 0 || index > HTML_LOG_COUNT) {
			return nullptr;
		}
		while (index >= static_cast<int>(html_logs.size())) {
			html_logs.emplace_back();
			initialize_html_log(html_logs.size() - 1);
		}
		return &html_logs[index];
	}

private:

	void initialize_html_log(int index) {
		html_logs[index].path = "debug-log-" + std::to_string(index) + ".html";
		file::write(html_logs[index].path, template_buffer);
	}

	void replace_substring(std::string& string, const std::string& substring, const std::string& replace_with) {
		auto index = string.find(substring);
		while (index != std::string::npos) {
			string.replace(index, substring.size(), replace_with);
			index = string.find(substring, index + replace_with.size());
		}
	}

	void write_field(int index, const std::string& message, int col_span = 1) {
		html_logs[index].final_buffer += "<td colspan=\"" + std::to_string(col_span) + "\">" + message + "</td>";
	}

	std::vector<html_log> html_logs;
	std::string template_buffer;

#endif

};

void append(int index, message_type type, const char* file, const char* func, int line, const std::string& message) {
#if ENABLE_HTML_LOG || ENABLE_STDOUT_LOG
	static logger_state logger;
	std::lock_guard lock{ logger.mutex };
#endif
#if ENABLE_HTML_LOG
	if (auto log = logger.get_html_log(index)) {
		log->temp_buffer = logger.get_html_compatible_string(message);
		logger.add(index, type, file, func, line);
	}
#endif
#if ENABLE_STDOUT_LOG
	std::cout << std::left << std::setw(13) << (current_local_time_string() + "." + current_time_ms_string()) << std::setw(1);
	std::cout << std::internal << type << ": " << message << "\n";
#endif
}

}

#endif
