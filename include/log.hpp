#pragma once

#include "platform.hpp"
#include "io.hpp"
#include "event.hpp"
#include "datetime.hpp"
#include "assert.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <mutex>
#include <optional>
#include <filesystem>

namespace nfwk::log {
enum class entry_type { message, warning, error, info };
}

// temporary until supported
namespace std {
class source_location {
public:
	std::string file_name() const {
		return "File";
	}
	std::string function_name() const {
		return "Function";
	}
	int line() const {
		return 1;
	}
	static source_location current() {
		return {};
	}
};
}

std::ostream& operator<<(std::ostream& out, nfwk::log::entry_type message);
std::ostream& operator<<(std::ostream& out, const std::vector<std::string>& strings);

namespace nfwk::log {

std::string current_local_time_string();
std::string curent_local_date_string();
std::string current_time_string_for_log();

class debug_log;

class log_writer {
public:

	log_writer(std::shared_ptr<debug_log> log) : log{ log } {}

	virtual ~log_writer();

	virtual void open() const {}
	virtual void flush() {}

	std::shared_ptr<debug_log> get_log() {
		return log;
	}

protected:
	
	std::shared_ptr<debug_log> log;

};

class log_entry {
public:

	entry_type type;
	std::string message;
	std::string file;
	std::string function;
	int line;
	std::string time;
	long long timestamp;

	log_entry(entry_type type, std::string_view message, std::string_view path, std::string_view function, int line)
		: type{ type }, message{ message }, file{ file_in_path(path) }, function{ function }, line{ line }, 
		time{ current_time_string_for_log() }, timestamp{ current_time<std::chrono::nanoseconds>() } {}

private:

	std::string_view file_in_path(std::string_view path) const {
		if (auto slash = path.rfind(std::filesystem::path::preferred_separator); slash != std::string::npos) {
			return path.substr(slash + 1);
		} else {
			return path;
		}
	}

};

class log_entry_identifier {
public:

	const char* const id;
	const std::source_location source;

	log_entry_identifier(const char* id, const std::source_location& source = std::source_location::current())
		: id{ id }, source{ source } {}

};

class debug_log {
public:

	event<const log_entry&> on_new_entry;

	const std::string name;

	bool show_time{ true };
	bool show_file{ true };
	bool show_line{ true };

	debug_log(std::string_view name) : name{ name } {}
	debug_log(std::string_view name, const std::vector<std::shared_ptr<debug_log>>& logs);

	int count() const {
		return static_cast<int>(entries.size());
	}

	const std::vector<log_entry>& get_entries() const {
		return entries;
	}

	template<typename... Args>
	void add(Args&&... args) {
		std::lock_guard lock{ mutex };
		const auto& entry = entries.emplace_back(std::forward<Args>(args)...);
		on_new_entry.emit(entry);
	}

	std::vector<std::shared_ptr<log_writer>> get_writers() const;

private:

	std::vector<log_entry> entries;
	std::mutex mutex;

};

void add_writer(std::shared_ptr<log_writer> writer);
void add_entry(const log_entry_identifier& identifier, entry_type type, std::string_view message);
std::shared_ptr<debug_log> find_log(const std::string& name);
std::vector<std::shared_ptr<debug_log>>& get_logs();

}

namespace nfwk {

template<typename... Args>
void message(log::log_entry_identifier id, std::string_view format, Args&&... args) {
	log::add_entry(id, log::entry_type::message, fmt::format(format, args...));
}

template<typename... Args>
void info(log::log_entry_identifier id, std::string_view format, Args&&... args) {
	log::add_entry(id, log::entry_type::info, fmt::format(format, args...));
}

template<typename... Args>
void warning(log::log_entry_identifier id, std::string_view format, Args&&... args) {
	log::add_entry(id, log::entry_type::warning, fmt::format(format, args...));
}

template<typename... Args>
void error(log::log_entry_identifier id, std::string_view format, Args&&... args) {
	log::add_entry(id, log::entry_type::error, fmt::format(format, args...));
}

}
