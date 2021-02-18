module;

#include <ctime>

export module nfwk.core:log.entry;

import std.core;
import std.filesystem;
import :datetime;

export namespace nfwk::log {

std::string current_local_time_string() {
	const std::time_t now{ std::time(nullptr) };
	tm local_time;
	localtime_s(&local_time, &now);
	char buffer[64];
	std::strftime(buffer, 64, "%X", &local_time);
	return buffer;
}

std::string curent_local_date_string() {
	const std::time_t now{ std::time(nullptr) };
	tm local_time;
	localtime_s(&local_time, &now);
	char buffer[64];
	std::strftime(buffer, 64, "%Y.%m.%d", &local_time);
	return buffer;
}

std::string current_time_string_for_log() {
	const auto ms = current_time<std::chrono::milliseconds>() % 1000;
	return current_local_time_string() + "." + std::to_string(ms);
}

enum class entry_type { message, warning, error, info };

class log_entry {
public:

	const entry_type type;
	const std::string message;
	const std::string file;
	const std::string function;
	const int line;
	const std::string time;

	log_entry(entry_type type, std::string_view message, std::string_view path, std::string_view function, int line)
		: type{ type }, message{ message }, file{ file_in_path(path) }, function{ function }, line{ line }, time{ current_time_string_for_log() } {}

private:

	std::string_view file_in_path(std::string_view path) const {
		if (auto slash = path.rfind(std::filesystem::path::preferred_separator); slash != std::string::npos) {
			return path.substr(slash + 1);
		} else {
			return path;
		}
	}

};

}

export std::ostream& operator<<(std::ostream& out, nfwk::log::entry_type message) {
	switch (message) {
	case nfwk::log::entry_type::message: return out << "message";
	case nfwk::log::entry_type::warning: return out << "warning";
	case nfwk::log::entry_type::error: return out << "error";
	case nfwk::log::entry_type::info: return out << "info";
	default: return out << "unknown";
	}
}
