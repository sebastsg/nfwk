#include "html_log_writer.hpp"
#include "assets.hpp"

namespace nfwk::log {

// this can be used for release builds to have fewer external files.
constexpr std::u8string_view default_template_html{
	u8"<!doctype html><html><head><style>"
	"body { margin: 0; background: #202021; }"
	"table { width: 100%; border-collapse: collapse; }"
	"td { border: 1px solid #333; background: #282828; color: #a0a0a0; font-family: monospace; }"
	".warning td { border-left: 2px solid #ff0; border-right: 2px solid #ff0; }"
	".critical td { border-left: 2px solid #f00; border-right: 2px solid #f00; }"
	".info td { border-left: 2px solid #14c8c8e6; border-right: 2px solid #14c8c8e6; }"
	"</head></style><body><table><tr class\"colhead\"><td>Time</td><td>Message</td>"
	"<td>File</td><td style=\"width:25%;\">Function</td><td>Line</td></tr>"
};

html_writer::html_writer(std::shared_ptr<debug_log> log) : log_writer{ log } {
	buffer = default_template_html;
	path = u8"logs/" + log->name + u8".html";
	for (const auto& entry : log->get_entries()) {
		buffer += entry_html(entry);
	}
	new_entry_event = log->on_new_entry.listen([this](const log_entry& entry) {
		buffer += entry_html(entry);
		flush();
	});
}

void html_writer::open() const {
	platform::open_file(path, false);
}

std::string html_writer::field_html(const std::u8string& message, int col_span) {
	return "<td colspan=\"" + std::to_string(col_span) + "\">" + std::string{ message.begin(), message.end() } + "</td>";
}

std::u8string html_writer::entry_html(const log_entry& entry) {
	std::stringstream html;
	html << "\r\n<tr class=\"" << entry.type << "\">";
	html << field_html(entry.time);
	html << field_html(html_compatible_string(entry.message));
	html << field_html(entry.file);
	html << field_html(html_compatible_string(entry.function));
	html << field_html(reinterpret_cast<const char8_t*>(std::to_string(entry.line).c_str()));
	html << "</tr>";
	return reinterpret_cast<const char8_t*>(html.str().c_str());
}

std::u8string html_writer::html_compatible_string(std::u8string string) {
	replace_substring(string, u8"&", u8"&amp;");
	replace_substring(string, u8">", u8"&gt;");
	replace_substring(string, u8"<", u8"&lt;");
	replace_substring(string, u8"\n", u8"<br>");
	replace_substring(string, u8"[b]", u8"<b>");
	replace_substring(string, u8"[/b]", u8"</b>");
	return string;
}

void html_writer::flush() {
	if (first_flush) {
		write_file(path, buffer);
		first_flush = false;
	} else {
		append_file(path, buffer);
	}
	buffer = u8"";
}

}
