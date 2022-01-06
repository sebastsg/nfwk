#pragma once

#include "platform.hpp"
#include "io.hpp"
#include "event.hpp"
#include "datetime.hpp"
#include "ansi_escape.hpp"

#include <mutex>
#include <filesystem>
#include <source_location>

namespace nfwk::log {
enum class entry_type { message, warning, error, info };
}

std::ostream& operator<<(std::ostream& out, nfwk::log::entry_type message);
std::ostream& operator<<(std::ostream& out, const std::vector<std::string>& strings);

namespace nfwk::log {

std::string current_local_time_string();
std::string current_local_date_string();
std::string current_time_string_for_log();

class debug_log;

class log_writer {
public:
	
	log_writer(std::shared_ptr<debug_log> log) : log{ std::move(log) } {}
	log_writer(const log_writer&) = delete;
	log_writer(log_writer&&) = delete;
	
	virtual ~log_writer();

	log_writer& operator=(const log_writer&) = delete;
	log_writer& operator=(log_writer&&) = delete;
	
	virtual void open() const {}
	virtual void flush() {}

	std::shared_ptr<debug_log> get_log() const {
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

	static std::string_view file_in_path(std::string_view path) {
		if (const auto slash = path.rfind(std::filesystem::path::preferred_separator); slash != std::string::npos) {
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

	consteval log_entry_identifier(const char* id, const std::source_location& source = std::source_location::current())
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

enum class text_graphics_formatting { none, ansi };

void format_text_graphics(std::string& string, text_graphics_formatting formatting);
void add_writer(const std::shared_ptr<log_writer>& new_writer);
void add_entry(const log_entry_identifier& identifier, entry_type type, std::string message);
std::shared_ptr<debug_log> find_log(std::string_view name);
std::vector<std::shared_ptr<debug_log>>& get_logs();

void add_writer_type(const std::function<std::shared_ptr<log_writer>(std::shared_ptr<debug_log>)>& make_writer);

template<typename Writer>
void add_writer_type() {
	add_writer_type([](std::shared_ptr<debug_log> log) {
		return std::make_shared<Writer>(log);
	});
}

}

namespace nfwk {

namespace core {
constexpr const auto* log = "core";
}

namespace draw {
constexpr const auto* log = "draw";
}

namespace audio {
constexpr const auto* log = "audio";
}

namespace scripts {
constexpr const auto* log = "scripts";
}

namespace network {
constexpr const auto* log = "network";
}

namespace graphics {
constexpr const auto* log = "graphics";
}

namespace ui {
constexpr const auto* log = "ui";
}

template<typename... Args>
void message(log::log_entry_identifier id, std::string_view format, Args&&... args) {
	log::add_entry(id, log::entry_type::message, "%white" + std::format(format, args...));
}

template<typename... Args>
void info(log::log_entry_identifier id, std::string_view format, Args&&... args) {
	log::add_entry(id, log::entry_type::info, "%light-blue" + std::format(format, args...));
}

template<typename... Args>
void warning(log::log_entry_identifier id, std::string_view format, Args&&... args) {
	log::add_entry(id, log::entry_type::warning, "%yellow" + std::format(format, args...));
}

template<typename... Args>
void error(log::log_entry_identifier id, std::string_view format, Args&&... args) {
	log::add_entry(id, log::entry_type::error, "%red" + std::format(format, args...));
}

template<typename... Args>
void bug(std::string_view format, Args&&... args) {
	log::add_entry("bugs", log::entry_type::warning, "%light-red" + std::format(format, args...));
}

template<typename... Args>
void log_errno(log::log_entry_identifier id, std::string_view format, Args&&... args) {
	char error_string[128]{};
	strerror_s(error_string, errno);
	log::add_entry(id, log::entry_type::error, "%red" + std::format(format, args...) + ". %redError: " + error_string);
}

}
