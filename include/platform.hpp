#pragma once

#include "graphics/surface.hpp"

#include <string>
#include <vector>
#include <filesystem>

namespace nfwk::platform {

#ifdef ENABLE_VERBOSE_LOGGING
#define LOG_VERBOSE_GL(GL_CALL) info("main", "{}", #GL_CALL);
#else
#define LOG_VERBOSE_GL(GL_CALL) 
#endif

#if 1
#define CHECK_GL_ERROR(GL_CALL) \
	GL_CALL; \
	LOG_VERBOSE_GL(GL_CALL) \
	if (const auto gl_error = glGetError(); gl_error != GL_NO_ERROR) { \
		error("graphics", "{}\n{}", #GL_CALL, gluErrorString(gl_error)); \
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

std::ostream& operator<<(std::ostream& out, nfwk::platform::system_cursor cursor);
