#include "platform.hpp"
#include "io.hpp"
#include "log.hpp"
#include "nfwk.hpp"

#include <Windows.h>
#include <ShObjIdl.h>
#include <Shlobj.h>

#include "windows_platform.hpp"

#include "timer.hpp"

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

std::u8string get_error_message(int error_code) {
	const int language{ MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) };
	char8_t* buffer{ nullptr };
	const auto flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER;
	FormatMessageA(flags, nullptr, error_code, language, reinterpret_cast<char*>(&buffer), 0, nullptr);
	if (buffer) {
		const std::u8string message{ buffer };
		LocalFree(buffer);
		return to_string(error_code) + u8": " + message;
	} else {
		return to_string(error_code);
	}
}

bool initialize_com() {
	if (const auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED); result != S_OK && result != S_FALSE) {
		nfwk::warning(nfwk::core::log, u8"Failed to initialize COM library.");
		return false;
	} else {
		return true;
	}
}

void uninitialize_com() {
	CoUninitialize();
}

}

namespace nfwk::platform {

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

void sleep(int ms) {
	Sleep(ms);
}

std::u8string environment_variable(std::u8string_view name) {
	char8_t buffer[2048];
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
	char8_t drive_letter{ 'A' };
	for (DWORD drive{ 1 }; drive_letter <= 'Z'; drive <<= 1, drive_letter++) {
		if (drives & drive) {
			paths.emplace_back(std::u8string{ drive_letter } + u8":/");
		}
	}
	return paths;
}

std::u8string open_file_browse_window() {
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
	return reinterpret_cast<const char8_t*>(file);
}

bool open_file_browser_and_select(std::filesystem::path path) {
	path.make_preferred();
	const auto items = ILCreateFromPathW(path.wstring().c_str());
	const auto result = SHOpenFolderAndSelectItems(items, 0, nullptr, 0);
	ILFree(items);
	if (result != S_OK) {
		warning(core::log, u8"Failed to open and select file: {}", path);
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
		warning(core::log, u8"Failed to create shell item: {}", path);
		return { 2, 2, pixel_format::rgba };
	}
	HBITMAP bitmap_handle{ nullptr };
	// todo: does E_PENDING work here? I tested it, but it seems useless.
	result = factory->GetImage({ scale, scale }, SIIGBF_RESIZETOFIT | SIIGBF_BIGGERSIZEOK, &bitmap_handle);
	if (result != S_OK) {
		warning(core::log, u8"Failed to load thumbnail: {}", path);
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
		warning(core::log, u8"Failed to open {}. Error: {}", path, status);
	}
}

std::vector<std::u8string> command_line_arguments() {
	std::vector<std::u8string> args;
	for (int i{ 0 }; i < __argc; i++) {
		args.emplace_back(reinterpret_cast<const char8_t*>(__argv[i]));
	}
	return args;
}

void relaunch() {
	info(core::log, u8"Relaunching program.");
	STARTUPINFO startup{};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process{};
	// todo: pass on arguments? need to implode argv into command line string.
	if (CreateProcess(__argv[0], nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startup, &process)) {
		// note: doesn't close process
		CloseHandle(process.hProcess);
		CloseHandle(process.hThread);
	} else {
		warning(core::log, u8"Failed to start {}. Error: {}", __argv[0], GetLastError());
	}
	// todo: exit event?
	std::exit(0);
}

}

int main() {
	if (!nfwk::platform::windows::initialize_com()) {
		return nfwk::return_code::windows_com_initialize_failed;
	}
	start();
	nfwk::platform::windows::uninitialize_com();
	return nfwk::return_code::success;
}

int WINAPI WinMain(HINSTANCE current_instance, HINSTANCE previous_instance, LPSTR command_line, int show_command) {
	nfwk::platform::windows::current_instance_arg = current_instance;
	nfwk::platform::windows::show_command_arg = show_command;
	return main();
}

std::ostream& operator<<(std::ostream& out, nfwk::platform::system_cursor cursor) {
	switch (cursor) {
	case nfwk::platform::system_cursor::none: return out << "None";
	case nfwk::platform::system_cursor::arrow: return out << "Arrow";
	case nfwk::platform::system_cursor::beam: return out << "Beam";
	case nfwk::platform::system_cursor::resize_all: return out << "Resize (all)";
	case nfwk::platform::system_cursor::resize_horizontal: return out << "Resize (horizontal)";
	case nfwk::platform::system_cursor::resize_vertical: return out << "Resize (vertical)";
	case nfwk::platform::system_cursor::resize_diagonal_from_bottom_left: return out << "Resize (bottom left -> top right)";
	case nfwk::platform::system_cursor::resize_diagonal_from_top_left: return out << "Resize (top left -> bottom right)";
	case nfwk::platform::system_cursor::block: return out << "Block";
	case nfwk::platform::system_cursor::hand: return out << "Hand";
	case nfwk::platform::system_cursor::help: return out << "Help";
	case nfwk::platform::system_cursor::cross: return out << "Cross";
	case nfwk::platform::system_cursor::wait: return out << "Wait";
	}
}
