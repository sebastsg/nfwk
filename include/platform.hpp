#pragma once

#include "graphics/surface.hpp"
#include "scoped_context.hpp"
#include "event.hpp"
#include "io.hpp"

#include <string>
#include <vector>
#include <filesystem>
#include <thread>
#include <atomic>

#define ENABLE_LINUX   0
#define ENABLE_WINDOWS 1

namespace nfwk {
class io_stream;
}

namespace nfwk::platform {

enum class supported_system { windows, linux };

constexpr supported_system current_system{
#if ENABLE_LINUX
	supported_system::linux
#elif ENABLE_WINDOWS
	supported_system::windows
#else
#error Platform not specified.
#endif
};
	
#ifdef ENABLE_VERBOSE_LOGGING
#define LOG_VERBOSE_GL(GL_CALL) {\
		info(core::log, "{}", #GL_CALL);\
	}
#else
#define LOG_VERBOSE_GL(GL_CALL) 
#endif

#if 1
#define CHECK_GL_ERROR(GL_CALL) \
	GL_CALL; \
	LOG_VERBOSE_GL(GL_CALL) \
	if (const auto gl_error = glGetError(); gl_error != GL_NO_ERROR) { \
		error(graphics::log, "{}\n{}", #GL_CALL, reinterpret_cast<const char*>(gluErrorString(gl_error))); \
		ASSERT(gl_error == GL_NO_ERROR); \
	}
#else
#define CHECK_GL_ERROR(GL_CALL) GL_CALL;
#endif

class system_command_runner {
public:

	system_command_runner(const std::string& command, std::function<void(io_stream*)> on_done);
	system_command_runner(const system_command_runner&) = delete;
	system_command_runner(system_command_runner&&) = delete;

	~system_command_runner();

	system_command_runner& operator=(const system_command_runner&) = delete;
	system_command_runner& operator=(system_command_runner&&) = delete;

	bool finish();

	bool is_done() const;
	io_stream* get_stream();

private:

	std::thread thread;
	io_stream stream;
	std::atomic<bool> done{ false };
	std::function<void(io_stream*)> on_done;
	bool finished{ false };
	
};

enum class system_cursor {
	none,
	arrow,
	beam,
	resize_all,
	resize_horizontal,
	resize_vertical,
	resize_diagonal_from_bottom_left,
	resize_diagonal_from_top_left,
	block,
	hand,
	help,
	cross,
	wait
};

void set_system_cursor(system_cursor cursor);

long long performance_frequency();
long long performance_counter();
void sleep(long long ms);

std::string environment_variable(std::string_view name);
bool is_system_file(std::filesystem::path path);

// on Windows, this will be C:/ etc.
std::vector<std::filesystem::path> get_root_directories();

// will block until a file is picked or window is closed
// todo: does this work similar on other platforms? might be a bad abstraction
std::string open_file_browse_window();

bool open_file_browser_and_select(std::filesystem::path path);

surface load_file_thumbnail(std::filesystem::path path, int scale);
void open_file(std::filesystem::path path, bool minimized);
void open_website_in_default_browser(std::string_view url);

std::vector<std::string> command_line_arguments();
void relaunch();

bool internal_thread_wrap_begin();
scoped_logic internal_thread_wrap_end();

template<typename ReturnType, typename Function>
ReturnType wrap_thread(Function function) {
	if (internal_thread_wrap_begin()) {
		const auto _ = internal_thread_wrap_end();
		return function();
	} else {
		return {};
	}
}

}

inline std::ostream& operator<<(std::ostream& out, nfwk::platform::system_cursor cursor) {
	switch (cursor) {
	using enum nfwk::platform::system_cursor;
	case none: return out << "None";
	case arrow: return out << "Arrow";
	case beam: return out << "Beam";
	case resize_all: return out << "Resize (all)";
	case resize_horizontal: return out << "Resize (horizontal)";
	case resize_vertical: return out << "Resize (vertical)";
	case resize_diagonal_from_bottom_left: return out << "Resize (bottom left -> top right)";
	case resize_diagonal_from_top_left: return out << "Resize (top left -> bottom right)";
	case block: return out << "Block";
	case hand: return out << "Hand";
	case help: return out << "Help";
	case cross: return out << "Cross";
	case wait: return out << "Wait";
	}
}

