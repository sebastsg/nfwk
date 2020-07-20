#pragma once

#include "platform.hpp"
#include "io.hpp"
#include "event.hpp"

#include <mutex>
#include <optional>

namespace no::debug::internal {
void start_debug();
void stop_debug();
}

namespace no::debug::log {

class log_writer {
public:

	const std::string name;

	log_writer(const std::string& name) : name{ name } {

	}

	virtual ~log_writer() = default;
	
	virtual void open() const {
	
	}

};

}

namespace no::debug::log::internal {
void add_writer(const std::string& name, std::unique_ptr<log_writer> writer);
}

namespace no::debug::log {

enum class message_type { message, warning, critical, info };

struct log_entry {

	const message_type type;
	const std::string message;
	std::string file;
	const std::string function;
	const int line;
	const std::string time;

	log_entry(message_type type, std::string_view message, std::string_view file, std::string_view function, int line);

};

class debug_log {
public:

	const std::string name;

	struct {
		event<const log_entry&> new_entry;
	} events;

	debug_log(const std::string& name);

	int count() const;
	const std::vector<log_entry>& entries() const;
	
	template<typename... Args>
	void add(Args&&... args) {
		std::lock_guard lock{ mutex };
		const auto& entry = log_entries.emplace_back(std::forward<Args>(args)...);
		events.new_entry.emit(entry);
	}

private:

	std::vector<log_entry> log_entries;
	std::mutex mutex;

};

class html_writer : public log_writer {
public:

	html_writer(debug_log& log);
	
	void open() const override;

private:
	
	static std::string entry_html(const log_entry& entry);
	static std::string field_html(const std::string& message, int col_span = 1);
	static std::string html_compatible_string(std::string string);

	void flush();

	std::string buffer;
	std::filesystem::path path;
	bool first_flush{ true };
	event_listener new_entry_event;

};

class stdout_writer : public log_writer {
public:

	stdout_writer(debug_log& log);

private:

	void write(const log_entry& entry);

	event_listener new_entry_event;

};

void add_entry(const std::string& name, message_type type, std::string_view file, std::string_view function, int line, std::string_view message);
std::optional<std::reference_wrapper<debug_log>> find_log(const std::string& name);

template<typename Writer>
void add_writer(const std::string& name) {
	if (auto log = find_log(name)) {
		internal::add_writer(name, std::make_unique<Writer>(log->get()));
	}
}

}

namespace no::debug::menu {

void enable();
void disable();
void update();
void add(std::string_view id, std::string_view name, const std::function<void()>& update);
void add(std::string_view id, const std::function<void()>& update);
void remove(std::string_view id);

}

#if ENABLE_DEBUG_LOG
#define BUG(MESSAGE) \
		WARNING(#MESSAGE); \
		abort();

#define ASSERT(EXPRESSION) \
		do { \
			if (!(EXPRESSION)) { \
				CRITICAL(#EXPRESSION); \
				abort(); \
			} \
		} while (0)

#if COMPILER_MSVC
# define DEBUG(ID, TYPE, STR) \
		no::debug::log::add_entry(ID, TYPE, __FILE__, __FUNCSIG__, __LINE__, STRING(STR))
#elif defined(COMPILER_GCC)
# define DEBUG(ID, TYPE, STR) \
		no::debug::log::add_entry(ID, TYPE, __FILE__, __PRETTY_FUNCTION__, __LINE__, STRING(STR))
#endif

# define DEBUG_LIMIT(ID, TYPE, STR, LIMIT) \
		{ \
			static int COUNTER{ 0 }; \
			if (++COUNTER <= (LIMIT)) { \
				DEBUG(ID, TYPE, "[" << COUNTER << "/" << LIMIT << "] " << STR); \
			} \
		}

#else
# define BUG(MESSAGE) 
# define ASSERT(EXPRESSION) 
# define DEBUG(ID, TYPE, STR) 
# define DEBUG_LIMIT(ID, TYPE, STR, LIMIT) 
#endif

#define MESSAGE(STR)  DEBUG("main", no::debug::log::message_type::message, STR)
#define WARNING(STR)  DEBUG("main", no::debug::log::message_type::warning, STR)
#define CRITICAL(STR) DEBUG("main", no::debug::log::message_type::critical, STR)
#define INFO(STR)     DEBUG("main", no::debug::log::message_type::info, STR)

#define MESSAGE_LIMIT(STR, LIMIT)  DEBUG_LIMIT("main", no::debug::log::message_type::message, STR, LIMIT)
#define WARNING_LIMIT(STR, LIMIT)  DEBUG_LIMIT("main", no::debug::log::message_type::warning, STR, LIMIT)
#define CRITICAL_LIMIT(STR, LIMIT) DEBUG_LIMIT("main", no::debug::log::message_type::critical, STR, LIMIT)
#define INFO_LIMIT(STR, LIMIT)     DEBUG_LIMIT("main", no::debug::log::message_type::info, STR, LIMIT)

#define MESSAGE_X(ID, STR)  DEBUG(ID, no::debug::log::message_type::message, STR)
#define WARNING_X(ID, STR)  DEBUG(ID, no::debug::log::message_type::warning, STR)
#define CRITICAL_X(ID, STR) DEBUG(ID, no::debug::log::message_type::critical, STR)
#define INFO_X(ID, STR)     DEBUG(ID, no::debug::log::message_type::info, STR)

#define MESSAGE_LIMIT_X(ID, STR, LIMIT)  DEBUG_LIMIT(ID, no::debug::log::message_type::message, STR, LIMIT)
#define WARNING_LIMIT_X(ID, STR, LIMIT)  DEBUG_LIMIT(ID, no::debug::log::message_type::warning, STR, LIMIT)
#define CRITICAL_LIMIT_x(ID, STR, LIMIT) DEBUG_LIMIT(ID, no::debug::log::message_type::critical, STR, LIMIT)
#define INFO_LIMIT_X(ID, STR, LIMIT)     DEBUG_LIMIT(ID, no::debug::log::message_type::info, STR, LIMIT)

std::ostream& operator<<(std::ostream& out, no::debug::log::message_type message);
