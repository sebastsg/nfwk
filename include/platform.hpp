#pragma once

#include <any>

#include "graphics/surface.hpp"
#include "scoped_context.hpp"

#include <string>
#include <vector>
#include <filesystem>
#include <functional>

namespace nfwk::platform {

#ifdef ENABLE_VERBOSE_LOGGING
#define LOG_VERBOSE_GL(GL_CALL) {\
		const char* expression = #GL_CALL;\
		info(core::log, u8"{}", reinterpret_cast<const char8_t*>(expression));\
	}
#else
#define LOG_VERBOSE_GL(GL_CALL) 
#endif

#if 1
#define CHECK_GL_ERROR(GL_CALL) \
	GL_CALL; \
	LOG_VERBOSE_GL(GL_CALL) \
	if (const auto gl_error = glGetError(); gl_error != GL_NO_ERROR) { \
		const char* expression = #GL_CALL;\
		error(graphics::log, u8"{}\n{}", reinterpret_cast<const char8_t*>(expression), gluErrorString(gl_error)); \
		ASSERT(gl_error == GL_NO_ERROR); \
	}
#else
#define CHECK_GL_ERROR(GL_CALL) GL_CALL;
#endif

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
void sleep(int ms);

std::u8string environment_variable(std::u8string_view name);
bool is_system_file(std::filesystem::path path);

// on Windows, this will be C:/ etc.
std::vector<std::filesystem::path> get_root_directories();

// will block until a file is picked or window is closed
// todo: does this work similar on other platforms? might be a bad abstraction
std::u8string open_file_browse_window();

bool open_file_browser_and_select(std::filesystem::path path);

surface load_file_thumbnail(std::filesystem::path path, int scale);
void open_file(std::filesystem::path path, bool minimized);

namespace windows {
bool initialize_com();
void uninitialize_com();
}

template<typename ReturnType, typename Function>
ReturnType wrap_thread(Function function) {
	if (!windows::initialize_com()) {
		return {};
	}
	scoped_logic uninitialize{ windows::uninitialize_com };
	return function();
}

std::vector<std::u8string> command_line_arguments();
void relaunch();

}

std::ostream& operator<<(std::ostream& out, nfwk::platform::system_cursor cursor);
