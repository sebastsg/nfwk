#include "platform.hpp"
#include "nfwk.hpp"
#include "io.hpp"
#include "log.hpp"

#if ENABLE_LINUX

namespace nfwk::platform {

static int in_argc;
static char** in_argv;

void set_system_cursor(system_cursor cursor) {
	
}

long long performance_frequency() {
	return 1;
}

long long performance_counter() {
	return 1;
}

void sleep(long long ms) {
	usleep(ms * 1000);
}

std::string environment_variable(std::string_view name) {
	return reinterpret_cast<const char*>(std::getenv(reinterpret_cast<const char*>(name.data())));
}

bool is_system_file(std::filesystem::path path) {
	return false;
}

std::vector<std::filesystem::path> get_root_directories() {
	return { "/" };
}

std::string open_file_browse_window() {
	return {};
}

bool open_file_browser_and_select(std::filesystem::path path) {
	
}

surface load_file_thumbnail(std::filesystem::path path, int scale) {
	return { 2, 2, pixel_format::rgba };
}

void open_file(std::filesystem::path path, bool minimized) {
	
}

io_stream system_output(std::string_view command) {
	info("core", "%green{}", command);
	FILE* process = popen(command.data(), "r");
	if (!process) {
		error("core", "Failed to run %cyan{}", command);
		return {};
	}
	io_stream stream;
	std::size_t last_read_size{ 0 };
	do {
		last_read_size = fread(stream.at_write(), 1, stream.size_left_to_write(), process);
		stream.move_write_index(last_read_size);
	} while (last_read_size != 0);
	pclose(process);
	return stream;
}

std::vector<std::string> command_line_arguments() {
	std::vector<std::string> args;
	for (int i{ 0 }; i < in_argc; i++) {
		args.emplace_back(reinterpret_cast<const char*>(in_argv[i]));
	}
	return args;
}

void relaunch() {
	info(core::log, "Relaunching program.");
	fork();
	std::exit(0);
}

bool internal_thread_wrap_begin() {
	return true;
}

scoped_logic internal_thread_wrap_end() {
	return {};
}

}

int main(int argc, char** argv) {
	nfwk::platform::in_argc = argc;
	nfwk::platform::in_argv = argv;
	start();
	return nfwk::return_code::success;
}

#endif
