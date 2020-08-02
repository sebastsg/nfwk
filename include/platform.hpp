#pragma once

#include "../config.hpp"
#include "graphics/surface.hpp"

#include <string>
#include <vector>
#include <filesystem>

#ifdef _MSC_VER
# define COMPILER_MSVC     1
# define COMPILER_GCC      0
#elif defined(__GNUC__)
# define COMPILER_MSVC     0
# define COMPILER_GCC      1
#else
# define COMPILER_MSVC     0
# define COMPILER_GCC      0
#endif

#ifdef _WIN32
# define PLATFORM_WINDOWS  1
# define PLATFORM_LINUX    0
#elif defined(__linux__)
# define PLATFORM_WINDOWS  0
# define PLATFORM_LINUX    1
#else
# define PLATFORM_WINDOWS  0
# define PLATFORM_LINUX    0
#endif

#define ENABLE_GL         (ENABLE_GRAPHICS && (PLATFORM_WINDOWS || PLATFORM_LINUX))
#define ENABLE_WASAPI     (ENABLE_AUDIO && PLATFORM_WINDOWS)
#define ENABLE_WINSOCK    (ENABLE_NETWORK && PLATFORM_WINDOWS)

#if COMPILER_MSVC
# define FORCE_INLINE __forceinline
#elif COMPILER_GCC
# define FORCE_INLINE __always_inline
#else
# define FORCE_INLINE 
#endif

namespace no::platform {

#if ENABLE_GL

#if ENABLE_DEBUG_LOG
# if ENABLE_VERBOSE_LOGGING
#  define LOG_VERBOSE_GL(GL_CALL) INFO(#GL_CALL);
# else
#  define LOG_VERBOSE_GL(GL_CALL) 
# endif
# define CHECK_GL_ERROR(GL_CALL) \
	GL_CALL; \
	LOG_VERBOSE_GL(GL_CALL) \
	if (const auto gl_error = glGetError(); gl_error != GL_NO_ERROR) { \
		CRITICAL_X("graphics", #GL_CALL << "\n" << gluErrorString(gl_error)); \
		ASSERT(gl_error == GL_NO_ERROR); \
	}
#else
# define CHECK_GL_ERROR(GL_CALL) GL_CALL;
#endif

#endif

#if PLATFORM_WINDOWS

class windows_window;
using platform_window = windows_window;

#if ENABLE_GL
class windows_gl_context;
using platform_render_context = windows_gl_context;
#endif

#elif PLATFORM_LINUX

class linux_window;
using platform_window = linux_window;

#if ENABLE_GL
class x11_gl_context;
using platform_render_context = x11_gl_context;
#endif

#endif

std::string current_local_time_string();
std::string curent_local_date_string();

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

std::string environment_variable(const std::string& name);
bool is_system_file(std::filesystem::path path);

// on Windows, this will be C:/ etc.
std::vector<std::filesystem::path> get_root_directories();

// will block until a file is picked or window is closed
// todo: does this work similar on other platforms? might be a bad abstraction
std::string open_file_browse_window();

bool open_file_browser_and_select(std::filesystem::path path);

surface load_file_thumbnail(std::filesystem::path path, int scale);
void open_file(std::filesystem::path path, bool minimized);

std::vector<std::string> command_line_arguments();
void relaunch();

}

std::ostream& operator<<(std::ostream& out, no::platform::system_cursor cursor);
