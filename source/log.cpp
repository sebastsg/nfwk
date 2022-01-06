#include "log.hpp"
#include "loop.hpp"
#include "graphics/ui.hpp"
#include "utility_functions.hpp"

#include <chrono>
#include <memory>
#include <typeinfo>
#include <array>

std::ostream& operator<<(std::ostream& out, nfwk::log::entry_type message) {
	switch (message) {
	case nfwk::log::entry_type::message: return out << "message";
	case nfwk::log::entry_type::warning: return out << "warning";
	case nfwk::log::entry_type::error: return out << "error";
	case nfwk::log::entry_type::info: return out << "info";
	}
}

std::ostream& operator<<(std::ostream& out, const std::vector<std::string>& strings) {
	for (std::size_t i{ 0 }; i < strings.size(); i++) {
		out << std::string{ strings[i].begin(), strings[i].end() };
		if (strings.size() - 1 > i) {
			out << ", ";
		}
	}
	return out;
}

namespace nfwk::ansi {

// { output, replacement length }
std::pair<std::string, std::size_t> format_ansi_text_graphics(std::string_view string, std::size_t offset) {
	// todo: c++20 constexpr
	static const std::vector<std::pair<std::string_view, std::string>> mapping{
		{ "reset", sgr::call(sgr::reset) },
		{ "bold", sgr::call(sgr::bold) },
		{ "italic", sgr::call(sgr::italic) },
		{ "underline", sgr::call(sgr::underline) },
		
		{ "black", sgr::call(sgr::black_text) },
		{ "red", sgr::call(sgr::red_text) },
		{ "green", sgr::call(sgr::green_text) },
		{ "yellow", sgr::call(sgr::yellow_text) },
		{ "blue", sgr::call(sgr::blue_text) },
		{ "magenta", sgr::call(sgr::magenta_text) },
		{ "cyan", sgr::call(sgr::cyan_text) },
		{ "white", sgr::call(sgr::white_text) },

		{ "light-black", sgr::call(sgr::bright_black_text) },
		{ "light-red", sgr::call(sgr::bright_red_text) },
		{ "light-green", sgr::call(sgr::bright_green_text) },
		{ "light-yellow", sgr::call(sgr::bright_yellow_text) },
		{ "light-blue", sgr::call(sgr::bright_blue_text) },
		{ "light-magenta", sgr::call(sgr::bright_magenta_text) },
		{ "light-cyan", sgr::call(sgr::bright_cyan_text) },
		{ "light-white", sgr::call(sgr::bright_white_text) },

		{ "black-background", sgr::call(sgr::black_background) },
		{ "red-background", sgr::call(sgr::red_background) },
		{ "green-background", sgr::call(sgr::green_background) },
		{ "yellow-background", sgr::call(sgr::yellow_background) },
		{ "blue-background", sgr::call(sgr::blue_background) },
		{ "magenta-background", sgr::call(sgr::magenta_background) },
		{ "cyan-background", sgr::call(sgr::cyan_background) },
		{ "white-background", sgr::call(sgr::white_background) },

		{ "light-black-background", sgr::call(sgr::bright_black_background) },
		{ "light-red-background", sgr::call(sgr::bright_red_background) },
		{ "light-green-background", sgr::call(sgr::bright_green_background) },
		{ "light-yellow-background", sgr::call(sgr::bright_yellow_background) },
		{ "light-blue-background", sgr::call(sgr::bright_blue_background) },
		{ "light-magenta-background", sgr::call(sgr::bright_magenta_background) },
		{ "light-cyan-background", sgr::call(sgr::bright_cyan_background) },
		{ "light-white-background", sgr::call(sgr::bright_white_background) }
	};
	for (const auto& [in, out] : mapping) {
		if (string.find(in, offset) == offset) {
			return { out, in.size() };
		}
	}
	return {};
}

}

namespace nfwk {

std::size_t format_none_text_graphics(std::string_view string, std::size_t offset) {
	static const std::vector<std::string_view> names{
		"reset", "bold", "italic", "underline",
		"black", "red", "green", "yellow", "blue", "magenta", "cyan", "white",
		"light-black", "light-red", "light-green", "light-yellow", "light-blue", "light-magenta", "light-cyan", "light-white",
		"black-background", "red-background", "green-background", "yellow-background", "blue-background-background", "magenta-background", "cyan-background", "white-background",
		"light-black-background", "light-red-background", "light-green-background", "light-yellow-background", "light-blue-background-background", "light-magenta-background", "light-cyan-background", "light-white-background"
	};
	for (const auto& name : names) {
		if (string.find(name, offset) == offset) {
			return name.size();
		}
	}
	return 0;
}

}

namespace nfwk::log {

static std::vector<std::shared_ptr<debug_log>> all_logs;
static std::vector<std::shared_ptr<log_writer>> writers;
static std::vector<std::function<std::shared_ptr<log_writer>(std::shared_ptr<debug_log>)>> writer_makers;

log_writer::~log_writer() {}

debug_log::debug_log(std::string_view name, const std::vector<std::shared_ptr<debug_log>>& logs) : name{ name } {
	for (const auto& log : logs) {
		entries = merge_vectors(entries, log->get_entries());
	}
	std::ranges::sort(entries, [](const log_entry& a, const log_entry& b) {
		return a.timestamp < b.timestamp;
	});
}

std::vector<std::shared_ptr<log_writer>> debug_log::get_writers() const {
	std::vector<std::shared_ptr<log_writer>> result;
	for (auto& writer : writers) {
		if (writer->get_log().get() == this) {
			result.push_back(writer);
		}
	}
	return result;
}

void add_writer(const std::shared_ptr<log_writer>& new_writer) {
	const auto& new_writer_type = typeid(*new_writer);
	auto it = std::ranges::find_if(writers, [&](const std::shared_ptr<log_writer>& existing_writer) {
		return existing_writer->get_log() == new_writer->get_log() && typeid(*existing_writer) == new_writer_type;
	});
	if (it == writers.end()) {
		writers.push_back(new_writer);
	}
}

void add_writer_type(const std::function<std::shared_ptr<log_writer>(std::shared_ptr<debug_log>)>& make_writer) {
	writer_makers.push_back(make_writer);
	for (auto& log : all_logs) {
		add_writer(make_writer(log));
	}
}

void add_log(std::string_view name) {
	for (const auto& log : all_logs) {
		if (log->name == name) {
			ASSERT(false); 
			return;
		}
	}
	auto& new_log = all_logs.emplace_back(std::make_shared<debug_log>(name));
	for (auto& make_writer : writer_makers) {
		add_writer(make_writer(new_log));
	}
}

void format_text_graphics(std::string& string, text_graphics_formatting formatting) {
	std::size_t offset{ 0 };
	auto begin = string.find('%', offset);
	while (begin != std::string::npos) {
		offset = begin + 1;
		switch (formatting) {
		case text_graphics_formatting::none:
			if (const auto& erase_size = format_none_text_graphics(string, offset); erase_size > 0) {
				string.erase(string.begin() + begin, string.begin() + begin + erase_size + 1);
			}
			break;
		case text_graphics_formatting::ansi:
			if (const auto& [as_ansi, replace_size] = ansi::format_ansi_text_graphics(string, offset); !as_ansi.empty()) {
				string.replace(string.begin() + begin, string.begin() + begin + replace_size + 1, as_ansi);
			}
			break;
		}
		begin = string.find('%', offset);
	}
	string += ansi::sgr::call(ansi::sgr::reset);
}

void add_entry(const log_entry_identifier& identifier, entry_type type, std::string message) {
	for (auto& log : all_logs) {
		if (log->name == identifier.id) {
			format_text_graphics(message, text_graphics_formatting::ansi);
			log->add(type, message, identifier.source.file_name(), identifier.source.function_name(), identifier.source.line());
			return;
		}
	}
	// log wasn't added, so we add the log and try again.
	add_log(identifier.id);
	add_entry(identifier, type, std::move(message));
}

std::vector<std::shared_ptr<debug_log>>& get_logs() {
	return all_logs;
}

std::shared_ptr<debug_log> find_log(std::string_view name) {
	for (auto& log : all_logs) {
		if (log->name == name) {
			return log;
		}
	}
	return nullptr;
}

std::string current_local_time_string() {
	const auto now = std::time(nullptr);
	tm local_time;
	localtime_s(&local_time, &now);
	char buffer[64]{};
	std::strftime(buffer, 64, "%X", &local_time);
	return buffer;
}

std::string current_local_date_string() {
	const auto now = std::time(nullptr);
	tm local_time;
	localtime_s(&local_time, &now);
	char buffer[64]{};
	std::strftime(buffer, 64, "%Y.%m.%d", &local_time);
	return buffer;
}

std::string current_time_string_for_log() {
	const auto ms = current_time<std::chrono::milliseconds>() % 1000;
	return current_local_time_string() + "." + std::to_string(ms);
}

}
