export module nfwk.core:log.html_writer;

import std.core;
import std.filesystem;
import :log.core;
import :events;
import :files;
import :string_functions;

// this can be used for release builds to have fewer external files.
constexpr std::string_view default_template_html{
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

export namespace nfwk::log {

class html_writer : public log_writer {
public:

	html_writer(debug_log& log) : log{ log } {
		buffer = template_html;
		//path = "logs/" + log.name + ".html";
		for (const auto& entry : log.get_entries()) {
			buffer += entry_html(entry);
		}
		new_entry_event = log.on_new_entry.listen([this](const log_entry& entry) {
			buffer += entry_html(entry);
			flush();
		});
	}

	void open() const override {
		//platform::open_file(path, false);
	}

private:

	static std::string field_html(const std::string& message, int col_span = 1) {
		return "<td colspan=\"" + std::to_string(col_span) + "\">" + message + "</td>";
	}

	static std::string entry_html(const log_entry& entry) {
		std::stringstream html;
		html << "\r\n<tr class=\"" << entry.type << "\">";
		html << field_html(entry.time);
		html << field_html(html_compatible_string(entry.message));
		html << field_html(entry.file);
		html << field_html(html_compatible_string(entry.function));
		html << field_html(std::to_string(entry.line));
		html << "</tr>";
		return html.str();
	}

	static std::string html_compatible_string(std::string string) {
		replace_substring(string, "&", "&amp;");
		replace_substring(string, ">", "&gt;");
		replace_substring(string, "<", "&lt;");
		replace_substring(string, "\n", "<br>");
		replace_substring(string, "[b]", "<b>");
		replace_substring(string, "[/b]", "</b>");
		return string;
	}

	void flush() {
		if (first_flush) {
			//write_file(path, buffer);
			first_flush = false;
		} else [[likely]] {
			//append_file(path, buffer);
		}
		buffer = "";
	}

	std::string buffer;
	std::filesystem::path path;
	bool first_flush{ true };
	event_listener new_entry_event;
	debug_log& log;

};

}