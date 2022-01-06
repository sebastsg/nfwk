#include "platform.hpp"
#include "log.hpp"
#include "nfwk.hpp"
#include "test.hpp"
#include "assert.hpp"

#include <Windows.h>
#include <ShObjIdl.h>
#include <ShlObj.h>

#include "windows_platform.hpp"

#include <codecvt>

#include "stdout_log_writer.hpp"

// these are defined by Windows when using vc++ runtime
extern int __argc;
extern char** __argv;

namespace nfwk::platform::windows {

static HINSTANCE current_instance_arg{ nullptr };
static int show_command_arg{ 0 };

HINSTANCE current_instance() {
	return current_instance_arg;
}

int show_command() {
	return show_command_arg;
}

bool initialize_com() {
	if (const auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED); result != S_OK && result != S_FALSE) {
		warning(core::log, "Failed to initialize COM library.");
		return false;
	} else {
		return true;
	}
}

void uninitialize_com() {
	CoUninitialize();
}

void initialize_console() {
	const auto stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD console_mode{ 0 };
	GetConsoleMode(stdout_handle, &console_mode);
	SetConsoleMode(stdout_handle, console_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
	SetConsoleOutputCP(65001);
}

std::string get_error_message(int error_code) {
	const int language{ MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) };
	char* buffer{ nullptr };
	const auto flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER;
	FormatMessageA(flags, nullptr, error_code, language, reinterpret_cast<char*>(&buffer), 0, nullptr);
	if (buffer) {
		const std::string message{ buffer };
		LocalFree(buffer);
		return std::to_string(error_code) + ": " + message;
	} else {
		return std::to_string(error_code);
	}
}

}

namespace nfwk::platform {

system_command_runner::system_command_runner(const std::string& command, std::function<void(io_stream*)> on_done_) : on_done{ std::move(on_done_) } {
	thread = std::thread{ [this, command] {
		//message("core", "Executing command: %green{}", command);
		if (!GetConsoleWindow()) {
			info("core", "Creating hidden console window to use _popen()");
			AllocConsole();
			ShowWindow(GetConsoleWindow(), SW_HIDE);
		}
		if (auto process = _popen(reinterpret_cast<const char*>(command.c_str()), "rb")) {
			std::size_t last_read_size{ 0 };
			bool try_again{ false };
			stream.allocate(32768);
			do {
				try_again = false;
				last_read_size = std::fread(stream.at_write(), 1, stream.size_left_to_write(), process);
				if (last_read_size != 0) {
					stream.move_write_index(last_read_size);
					stream.resize_if_needed(2048);
				} else {
					if (ferror(process) != 0) {
						stream.reset();
					} else if (feof(process) == 0) {
						try_again = true;
					}
				}
			} while (last_read_size != 0 || try_again);
			_pclose(process);
		} else {
			error("core", "Failed to run command: %green{}", command);
		}
		done = true;
	} };
}

system_command_runner::~system_command_runner() {
	if (thread.joinable()) {
		thread.join();
	}
}

bool system_command_runner::finish() {
	ASSERT(!finished);
	if (finished) {
		return true;
	} else if (done) {
		if (on_done) {
			on_done(&stream);
		}
		finished = true;
		return true;
	} else {
		return false;
	}
}

bool system_command_runner::is_done() const {
	return done;
}

io_stream* system_command_runner::get_stream() {
	return &stream;
}

static LPCSTR get_system_cursor_resource(system_cursor cursor) {
	switch (cursor) {
	case system_cursor::arrow: return IDC_ARROW;
	case system_cursor::beam: return IDC_IBEAM;
	case system_cursor::resize_all: return IDC_SIZEALL;
	case system_cursor::resize_horizontal: return IDC_SIZEWE;
	case system_cursor::resize_vertical: return IDC_SIZENS;
	case system_cursor::resize_diagonal_from_bottom_left: return IDC_SIZENESW;
	case system_cursor::resize_diagonal_from_top_left: return IDC_SIZENWSE;
	case system_cursor::block: return IDC_NO;
	case system_cursor::hand: return IDC_HAND;
	case system_cursor::help: return IDC_HELP;
	case system_cursor::cross: return IDC_CROSS;
	case system_cursor::wait: return IDC_WAIT;
	default: return IDC_ARROW;
	}
}

void set_system_cursor(system_cursor cursor) {
	SetCursor(LoadCursor(nullptr, get_system_cursor_resource(cursor)));
}

long long performance_frequency() {
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return frequency.QuadPart;
}

long long performance_counter() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart;
}

void sleep(long long ms) {
	Sleep(static_cast<DWORD>(ms));
}

std::string environment_variable(std::string_view name) {
	char buffer[2048];
	GetEnvironmentVariableA(reinterpret_cast<const char*>(name.data()), reinterpret_cast<char*>(buffer), sizeof(buffer));
	return buffer;
}

bool is_system_file(std::filesystem::path path) {
	path.make_preferred();
	const auto string = std::wstring{ L"\\\\?\\" } + path.wstring();
	return GetFileAttributesW(string.c_str()) & FILE_ATTRIBUTE_SYSTEM;
}

std::vector<std::filesystem::path> get_root_directories() {
	std::vector<std::filesystem::path> paths;
	const DWORD drives{ GetLogicalDrives() };
	char drive_letter{ 'A' };
	for (DWORD drive{ 1 }; drive_letter <= 'Z'; drive <<= 1, drive_letter++) {
		if (drives & drive) {
			paths.emplace_back(std::string{ drive_letter } + ":/");
		}
	}
	return paths;
}

std::string open_file_browse_window() {
	char file[MAX_PATH]{};
	char file_title[MAX_PATH]{};
	char template_name[MAX_PATH]{};
	OPENFILENAME data{};
	data.lStructSize = sizeof(data);
	data.lpstrFile = file;
	data.nMaxFile = MAX_PATH;
	data.lpstrFileTitle = file_title;
	data.lpTemplateName = template_name;
	data.lpstrFilter = "All\0*.*\0";
	data.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	GetOpenFileNameA(&data);
	return reinterpret_cast<const char*>(file);
}

bool open_file_browser_and_select(std::filesystem::path path) {
	path.make_preferred();
	const auto items = ILCreateFromPathW(path.wstring().c_str());
	const auto result = SHOpenFolderAndSelectItems(items, 0, nullptr, 0);
	ILFree(items);
	if (result != S_OK) {
		warning(core::log, "Failed to open and select file: {}", path);
	}
	return result == S_OK;
}

surface bitmap_as_surface(HBITMAP bitmap_handle) {
	DIBSECTION dib_section{};
	GetObject(bitmap_handle, sizeof(DIBSECTION), &dib_section);
	const BITMAP& bitmap{ dib_section.dsBm };
	auto pixels = static_cast<std::uint32_t*>(bitmap.bmBits);
	surface result{ pixels, bitmap.bmWidth, bitmap.bmHeight, pixel_format::bgra, surface::construct_by::copy };
	// todo: figure out when bitmap is flipped. docs are a bit confusing.
	//if (dib_section.dsBmih.biHeight < 0) {
	//	result.flip_vertically();
	//}
	return result;
}

surface load_file_thumbnail(std::filesystem::path path, int scale) {
	path.make_preferred();
	IShellItemImageFactory* factory{ nullptr };
	auto result = SHCreateItemFromParsingName(path.wstring().c_str(), nullptr, IID_PPV_ARGS(&factory));
	if (result != S_OK) {
		warning(core::log, "Failed to create shell item: {}", path);
		return { 2, 2, pixel_format::rgba };
	}
	HBITMAP bitmap_handle{ nullptr };
	// todo: does E_PENDING work here? I tested it, but it seems useless.
	result = factory->GetImage({ scale, scale }, SIIGBF_RESIZETOFIT | SIIGBF_BIGGERSIZEOK, &bitmap_handle);
	if (result != S_OK) {
		warning(core::log, "Failed to load thumbnail: {}", path);
		return { 2, 2, pixel_format::rgba };
	}
	surface thumbnail{ bitmap_as_surface(bitmap_handle) };
	DeleteObject(bitmap_handle);
	return thumbnail;
}

void open_file(std::filesystem::path path, bool minimized) {
	path.make_preferred();
	const auto show = minimized ? SW_HIDE : SW_SHOW;
	const auto status = reinterpret_cast<int>(ShellExecuteW(nullptr, nullptr, path.wstring().c_str(), nullptr, nullptr, show));
	if (status <= 32) {
		warning(core::log, "Failed to open file: {}. Error: {}", path, status);
	}
}

std::wstring u8string_to_wstring(std::string_view u8) {
	// todo: have thread_local buffer/io_stream to prevent superfluous allocations.
	const auto max_size = static_cast<int>(u8.size()) * 8;
	auto buffer = new wchar_t[max_size]{};
	const auto written_size = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(u8.data()), u8.size(), buffer, max_size);
	if (written_size == 0) {
		// const auto error = GetLastError();
	}
	std::wstring result{ buffer, static_cast<std::size_t>(written_size) };
	delete[] buffer;
	return result;
}

void open_website_in_default_browser(std::string_view u8_url) {
	const auto wide_url = u8string_to_wstring(u8_url);
	const auto status = reinterpret_cast<int>(ShellExecuteW(nullptr, nullptr, wide_url.c_str(), nullptr, nullptr, SW_SHOW));
	if (status <= 32) {
		warning(core::log, "Failed to open website: {}. Error: {}", u8_url, status);
	}
}

std::vector<std::string> command_line_arguments() {
	std::vector<std::string> args;
	for (int i{ 0 }; i < __argc; i++) {
		args.emplace_back(reinterpret_cast<const char*>(__argv[i]));
	}
	return args;
}

void relaunch() {
	info(core::log, "Relaunching program.");
	STARTUPINFO startup{};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process{};
	// todo: pass on arguments? need to implode argv into command line string.
	if (CreateProcessA(__argv[0], nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startup, &process)) {
		// note: doesn't close process
		CloseHandle(process.hProcess);
		CloseHandle(process.hThread);
	} else {
		warning(core::log, "Failed to start {}. Error: {}", __argv[0], GetLastError());
	}
	// todo: exit event?
	std::exit(0);
}

bool internal_thread_wrap_begin() {
	return windows::initialize_com();
}

scoped_logic internal_thread_wrap_end() {
	return windows::uninitialize_com;
}

}

static int nfwk_main() {
	if (!nfwk::platform::windows::initialize_com()) {
		return nfwk::return_code::windows_com_initialize_failed;
	}
	if (nfwk::test::run_tests()) {
		start();
	} else {
		nfwk::log::add_writer_type<nfwk::log::stdout_writer>();
		system("pause");
	}
	nfwk::platform::windows::uninitialize_com();
	return nfwk::return_code::success;
}

int main() {
	return nfwk_main();
}

int WINAPI WinMain(HINSTANCE current_instance, HINSTANCE previous_instance, LPSTR command_line, int show_command) {
	nfwk::platform::windows::current_instance_arg = current_instance;
	nfwk::platform::windows::show_command_arg = show_command;
	return nfwk_main();
}
