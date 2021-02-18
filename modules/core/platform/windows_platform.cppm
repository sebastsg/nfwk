module;

#include <Windows.h>
#include <ShObjIdl.h>
#include <Shlobj.h>

export module nfwk.core:platform.windows;

import std.core;
import std.filesystem;
import :log;
export import :platform.base;

// these are defined by Windows when using vc++ runtime
extern int __argc;
extern char** __argv;

namespace nfwk {

LPCSTR get_system_cursor_resource(system_cursor cursor) {
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

}

export namespace nfwk {

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

#if 0
bool is_system_file(std::filesystem::path path) {
	path.make_preferred();
	auto string = std::wstring{ L"\\\\?\\" } + path.wstring();
	return GetFileAttributesW(string.c_str()) & FILE_ATTRIBUTE_SYSTEM;
}
#endif

std::string environment_variable(const std::string& name) {
	char buffer[2048];
	GetEnvironmentVariableA(name.c_str(), buffer, 2048);
	return buffer;
}

// on Windows, this will be C:/ etc.
#if 0
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
#endif

// will block until a file is picked or window is closed
// todo: does this work similar on other platforms? might be a bad abstraction
std::string open_file_browse_window() {
	char file[MAX_PATH]{};
	char file_title[MAX_PATH]{};
	char template_name[MAX_PATH]{};
	OPENFILENAME data{};
	data.lStructSize = sizeof(data);
	data.lpstrFile = file;
	data.lpstrFileTitle = file_title;
	data.lpTemplateName = template_name;
	data.nMaxFile = MAX_PATH;
	data.lpstrFilter = "All\0*.*\0";
	data.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	GetOpenFileName(&data);
	return file;
}

#if 0
bool open_file_browser_and_select(std::filesystem::path path) {
	path.make_preferred();
	auto items = ILCreateFromPathW(path.wstring().c_str());
	auto result = SHOpenFolderAndSelectItems(items, 0, nullptr, 0);
	ILFree(items);
	if (result != S_OK) {
		warning("platform", "Failed to open and select file: {}", path);
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
		warning("platform", "Failed to create shell item: {}", path);
		return { 2, 2, pixel_format::rgba };
	}
	HBITMAP bitmap_handle{ nullptr };
	do {
		result = factory->GetImage({ scale, scale }, SIIGBF_RESIZETOFIT | SIIGBF_BIGGERSIZEOK, &bitmap_handle);
	} while (result == E_PENDING);
	if (result != S_OK) {
		warning("platform", "Failed to load thumbnail: {}", path);
		return { 2, 2, pixel_format::rgba };
	}
	surface thumbnail{ bitmap_as_surface(bitmap_handle) };
	DeleteObject(bitmap_handle);
	return thumbnail;
}
void open_file(std::filesystem::path path, bool minimized) {
	path.make_preferred();
	const auto show = minimized ? SW_HIDE : SW_SHOW;
	const auto status = reinterpret_cast<int>(ShellExecute(nullptr, nullptr, path.string().c_str(), nullptr, nullptr, show));
	if (status <= 32) {
		warning("platform", "Failed to open {}. Status: {}", path, status);
	}
}
#endif

std::vector<std::string> command_line_arguments() {
	std::vector<std::string> args;
	for (int i{ 0 }; i < __argc; i++) {
		args.push_back(__argv[i]);
	}
	return args;
}

void relaunch() {
	STARTUPINFO startup{};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process{};
	// todo: pass on arguments? need to implode argv into command line string.
	if (CreateProcess(__argv[0], nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startup, &process)) {
		// note: doesn't close process
		CloseHandle(process.hProcess);
		CloseHandle(process.hThread);
	} else {
		warning("platform", "Failed to start {}. Error: {}", __argv[0], GetLastError());
	}
	//destroy_main_loop();
	std::exit(0);
}

}
