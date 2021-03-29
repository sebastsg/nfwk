#pragma once

#include "platform.hpp"
#include "io.hpp"
#include "event.hpp"
#include "datetime.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <mutex>
#include <filesystem>

namespace nfwk::log {
enum class entry_type { message, warning, error, info };
}

// temporary until supported
namespace std {
class source_location {
public:
	std::u8string file_name() const {
		return u8"File";
	}
	std::u8string function_name() const {
		return u8"Function";
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
std::ostream& operator<<(std::ostream& out, const std::vector<std::u8string>& strings);

namespace nfwk::log {

std::u8string current_local_time_string();
std::u8string current_local_date_string();
std::u8string current_time_string_for_log();

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
	std::u8string message;
	std::u8string file;
	std::u8string function;
	int line;
	std::u8string time;
	long long timestamp;

	log_entry(entry_type type, std::u8string_view message, std::u8string_view path, std::u8string_view function, int line)
		: type{ type }, message{ message }, file{ file_in_path(path) }, function{ function }, line{ line }, 
		time{ current_time_string_for_log() }, timestamp{ current_time<std::chrono::nanoseconds>() } {}

private:

	static std::u8string_view file_in_path(std::u8string_view path) {
		if (const auto slash = path.rfind(std::filesystem::path::preferred_separator); slash != std::u8string::npos) {
			return path.substr(slash + 1);
		} else {
			return path;
		}
	}

};

class log_entry_identifier {
public:

	const char8_t* const id;
	const std::source_location source;

	log_entry_identifier(const char* id, const std::source_location& source = std::source_location::current())
		: id{ reinterpret_cast<const char8_t*>(id) }, source{ source } {}

#ifdef NFWK_CPP_20
	log_entry_identifier(const char8_t* id, const std::source_location& source = std::source_location::current())
		: id{ id }, source{ source } {}
#endif

};

class debug_log {
public:

	event<const log_entry&> on_new_entry;

	const std::u8string name;

	bool show_time{ true };
	bool show_file{ true };
	bool show_line{ true };

	debug_log(std::u8string_view name) : name{ name } {}
	debug_log(std::u8string_view name, const std::vector<std::shared_ptr<debug_log>>& logs);

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

void add_writer(const std::shared_ptr<log_writer>& new_writer);
void add_entry(const log_entry_identifier& identifier, entry_type type, std::u8string_view message);
std::shared_ptr<debug_log> find_log(const std::u8string& name);
std::vector<std::shared_ptr<debug_log>>& get_logs();

}

namespace nfwk {

namespace core {
constexpr const auto* log = u8"core";
}

namespace draw {
constexpr const auto* log = u8"draw";
}

namespace audio {
constexpr const auto* log = u8"audio";
}

namespace scripts {
constexpr const auto* log = u8"scripts";
}

namespace network {
constexpr const auto* log = u8"network";
}

namespace graphics {
constexpr const auto* log = u8"graphics";
}

namespace ui {
constexpr const auto* log = u8"ui";
}

template<typename... Args>
void message(log::log_entry_identifier id, std::u8string_view format, Args&&... args) {
	log::add_entry(id, log::entry_type::message, fmt::format(format, args...));
}

template<typename... Args>
void info(log::log_entry_identifier id, std::u8string_view format, Args&&... args) {
	log::add_entry(id, log::entry_type::info, fmt::format(format, args...));
}

template<typename... Args>
void warning(log::log_entry_identifier id, std::u8string_view format, Args&&... args) {
	log::add_entry(id, log::entry_type::warning, fmt::format(format, args...));
}

template<typename... Args>
void error(log::log_entry_identifier id, std::u8string_view format, Args&&... args) {
	log::add_entry(id, log::entry_type::error, fmt::format(format, args...));
}

template<typename... Args>
void bug(std::u8string_view format, Args&&... args) {
	log::add_entry(u8"bugs", log::entry_type::warning, fmt::format(format, args...));
}

}
