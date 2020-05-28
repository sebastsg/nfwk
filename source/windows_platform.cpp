#include "platform.hpp"
#include "io.hpp"
#include "debug.hpp"
#include "loop.hpp"

#if PLATFORM_WINDOWS

#include <Windows.h>
#include <ShObjIdl.h>

#include "windows_platform.hpp"

// these are defined by Windows when using vc++ runtime
extern int __argc;
extern char** __argv;

namespace no::platform::windows {

static HINSTANCE current_instance_arg{ nullptr };
static int show_command_arg{ 0 };

HINSTANCE current_instance() {
	return current_instance_arg;
}

int show_command() {
	return show_command_arg;
}

}

namespace no::platform {

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

std::string environment_variable(const std::string& name) {
	char buffer[2048];
	GetEnvironmentVariableA(name.c_str(), buffer, 2048);
	return buffer;
}

bool is_system_file(const std::filesystem::path& path) {
	return GetFileAttributes(path.string().c_str()) & FILE_ATTRIBUTE_SYSTEM;
}

std::string open_file_browse_window() {
	char file[MAX_PATH];
	char file_title[MAX_PATH];
	char template_name[MAX_PATH];
	OPENFILENAME data{};
	data.lStructSize = sizeof(data);
	data.hwndOwner = nullptr;
	data.lpstrDefExt = nullptr;
	data.lpstrCustomFilter = nullptr;
	data.lpstrFile = file;
	data.lpstrFile[0] = '\0';
	data.nMaxFile = MAX_PATH;
	data.lpstrFileTitle = file_title;
	data.lpstrFileTitle[0] = '\0';
	data.lpTemplateName = template_name;
	data.nMaxFileTitle = 0;
	data.lpstrFilter = "All\0*.*\0";
	data.nFilterIndex = 0;
	data.lpstrInitialDir = nullptr;
	data.lpstrTitle = nullptr;
	data.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	GetOpenFileName(&data);
	return file;
}

surface bitmap_as_surface(HBITMAP bitmap_handle) {
	DIBSECTION dib_section{};
	GetObject(bitmap_handle, sizeof(DIBSECTION), &dib_section);
	const BITMAP& bitmap{ dib_section.dsBm };
	auto pixels = static_cast<uint32_t*>(bitmap.bmBits);
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
	HRESULT result = SHCreateItemFromParsingName(path.wstring().c_str(), nullptr, IID_PPV_ARGS(&factory));
	if (result != S_OK) {
		WARNING("Failed to create shell item from path: " << path);
		return { 2, 2, pixel_format::rgba };
	}
	HBITMAP bitmap_handle{ nullptr };
	result = factory->GetImage({ scale, scale }, SIIGBF_RESIZETOFIT, &bitmap_handle);
	if (result != S_OK) {
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
		WARNING("Failed to open " << path << ". Error: " << status);
	}
}

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
	const auto success{ CreateProcess(__argv[0], nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startup, &process) };
	if (success) {
		// note: doesn't close process
		CloseHandle(process.hProcess);
		CloseHandle(process.hThread);
	} else {
		WARNING("Failed to start " << __argv[0] << ". Error: " << GetLastError());
	}
	internal::destroy_main_loop();
	std::exit(0);
}

}

#if ENABLE_WINDOW

int WINAPI WinMain(HINSTANCE current_instance, HINSTANCE previous_instance, LPSTR command_line, int show_command) {
	no::platform::windows::current_instance_arg = current_instance;
	no::platform::windows::show_command_arg = show_command;
	if (const auto result = CoInitialize(nullptr); result != S_OK && result != S_FALSE) {
		WARNING("Failed to run CoInitialize.");
	}
	const int result{ no::internal::run_main_loop() };
	CoUninitialize();
	return result;
}

#else

int main() {
	CoInitialize(nullptr);
	int result = no::internal::run_main_loop();
	CoUninitialize();
	return result;
}

#endif

#endif

std::ostream& operator<<(std::ostream& out, no::platform::system_cursor cursor) {
	switch (cursor) {
	case no::platform::system_cursor::none: return out << "None";
	case no::platform::system_cursor::arrow: return out << "Arrow";
	case no::platform::system_cursor::beam: return out << "Beam";
	case no::platform::system_cursor::resize_all: return out << "Resize (all)";
	case no::platform::system_cursor::resize_horizontal: return out << "Resize (horizontal)";
	case no::platform::system_cursor::resize_vertical: return out << "Resize (vertical)";
	case no::platform::system_cursor::resize_diagonal_from_bottom_left: return out << "Resize (bottom left -> top right)";
	case no::platform::system_cursor::resize_diagonal_from_top_left: return out << "Resize (top left -> bottom right)";
	case no::platform::system_cursor::block: return out << "Block";
	case no::platform::system_cursor::hand: return out << "Hand";
	case no::platform::system_cursor::help: return out << "Help";
	case no::platform::system_cursor::cross: return out << "Cross";
	case no::platform::system_cursor::wait: return out << "Wait";
	default: return out << "Invalid";
	}
}
