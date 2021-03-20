#include "windows_window.hpp"

#include <Windows.h>
#include <windowsx.h>

#include "graphics/window.hpp"
#include "input.hpp"
#include "vector4.hpp"
#include "log.hpp"

static nfwk::vector2i current_mouse_position;

LRESULT WINAPI process_window_messages(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) {
	auto active_window = reinterpret_cast<nfwk::platform::windows_window*>(GetWindowLongPtr(window_handle, 0));
	if (!active_window) {
		return DefWindowProc(window_handle, message, w_param, l_param);
	}
	auto& keyboard = active_window->keyboard;
	auto& mouse = active_window->mouse;
	switch (message) {
	case WM_PAINT:
		return DefWindowProc(window_handle, message, w_param, l_param);
	case WM_INPUT:
		return 0;
	case WM_MOUSEMOVE:
	{
		const int new_x{ GET_X_LPARAM(l_param) };
		const int new_y{ GET_Y_LPARAM(l_param) };
		const int relative_x{ new_x - current_mouse_position.x };
		const int relative_y{ new_y - current_mouse_position.y };
		if (relative_x == 0 && relative_y == 0) {
			return 0;
		}
		current_mouse_position = { new_x, new_y };
		mouse.move.emit(nfwk::vector2i{ relative_x, relative_y }, nfwk::vector2i{ new_x, new_y });
		return 0;
	}
	case WM_LBUTTONDOWN:
		mouse.press.emit(nfwk::mouse::button::left);
		return 0;
	case WM_LBUTTONUP:
		mouse.release.emit(nfwk::mouse::button::left);
		return 0;
	case WM_LBUTTONDBLCLK:
		mouse.double_click.emit(nfwk::mouse::button::left);
		return 0;
	case WM_RBUTTONDOWN:
		mouse.press.emit(nfwk::mouse::button::right);
		return 0;
	case WM_RBUTTONUP:
		mouse.release.emit(nfwk::mouse::button::right);
		return 0;
	case WM_RBUTTONDBLCLK:
		mouse.double_click.emit(nfwk::mouse::button::right);
		return 0;
	case WM_MBUTTONDOWN:
		mouse.press.emit(nfwk::mouse::button::middle);
		return 0;
	case WM_MBUTTONUP:
		mouse.release.emit(nfwk::mouse::button::middle);
		return 0;
	case WM_MBUTTONDBLCLK:
		mouse.double_click.emit(nfwk::mouse::button::middle);
		return 0;
	case WM_MOUSEWHEEL:
		// TODO: Maybe this won't work for mouse wheels without notches.
		mouse.scroll.emit(GET_WHEEL_DELTA_WPARAM(w_param) / WHEEL_DELTA);
		return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		const LPARAM repeat{ l_param >> 30 };
		nfwk::key key{ static_cast<nfwk::key>(w_param) };
		if (w_param == VK_SHIFT) {
			key = ((GetKeyState(VK_LSHIFT) & 0x8000) != 0 ? nfwk::key::left_shift : nfwk::key::right_shift);
		} else if (w_param == VK_CONTROL) {
			key = ((GetKeyState(VK_LCONTROL) & 0x8000) != 0 ? nfwk::key::left_control : nfwk::key::right_control);
		}
		keyboard.repeated_press.emit(key);
		if (repeat == 0) {
			keyboard.press.emit(key);
		}
		return 0;
	}
	case WM_SYSKEYUP:
	case WM_KEYUP:
	{
		nfwk::key key{ static_cast<nfwk::key>(w_param) };
		if (w_param == VK_SHIFT) {
			if (keyboard.is_key_down(nfwk::key::left_shift) && (GetKeyState(VK_LSHIFT) & 0x8000) == 0) {
				key = nfwk::key::left_shift;
			} else {
				key = nfwk::key::right_shift;
			}
		} else if (w_param == VK_CONTROL) {
			if (keyboard.is_key_down(nfwk::key::left_control) && (GetKeyState(VK_LCONTROL) & 0x8000) == 0) {
				key = nfwk::key::left_control;
			} else {
				key = nfwk::key::right_control;
			}
		}
		keyboard.release.emit(key);
		return 0;
	}
	case WM_CHAR:
		keyboard.input.emit(static_cast<unsigned int>(w_param));
		return 0;
	case WM_SIZE:
		active_window->on_resize.emit<int, int>(LOWORD(l_param), HIWORD(l_param));
		return 0;
	case WM_SETCURSOR:
		if (LOWORD(l_param) == HTCLIENT && mouse.icon.total_listeners() > 0) {
			mouse.icon.emit();
			return 1;
		}
		return DefWindowProc(window_handle, message, w_param, l_param);
	case WM_ERASEBKGND:
		return 1;
	case WM_CLOSE:
		active_window->on_close.emit();
		return 0;
	default:
		return DefWindowProc(window_handle, message, w_param, l_param);
	}
}

namespace nfwk::platform {

static bool create_window_class(WNDPROC procedure, std::string_view name) {
	message("draw", "Registering window class: {}", name);
	WNDCLASS window{};
	window.style = CS_OWNDC | CS_DBLCLKS;
	window.lpfnWndProc = procedure;
	window.cbClsExtra = 0;
	window.cbWndExtra = sizeof(LONG_PTR);
	window.hInstance = windows::current_instance();
	window.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	window.hCursor = LoadCursor(NULL, IDC_ARROW);
	window.hbrBackground = nullptr;
	window.lpszMenuName = nullptr;
	window.lpszClassName = name.data();
	if (RegisterClass(&window)) {
		return true;
	} else {
		error("draw", "Failed to register window class");
		return false;
	}
}

static vector2i calculate_actual_window_size(int width, int height, DWORD style) {
	RECT area{ 0, 0, width, height };
	AdjustWindowRect(&area, style, false);
	return { area.right - area.left, area.bottom - area.top };
}

static vector4i calculate_maximized_window_rectangle() {
	RECT area{};
	SystemParametersInfo(SPI_GETWORKAREA, 0, &area, 0);
	return { area.left, area.top, area.right, area.bottom };
}

static void destroy_window(HWND window_handle, HDC device_context) {
	if (window_handle) {
		message("draw", "Destroying window");
		ReleaseDC(window_handle, device_context);
		DestroyWindow(window_handle);
	}
}

void windows_window::create_classes() {
	create_window_class(DefWindowProc, "compatibility");
	create_window_class(process_window_messages, "main");
}

HWND windows_window::create_window(std::string_view name, std::string_view type, int width, int height, bool maximized) {
	message("draw", "Creating \"{}\" window \"{}\"", type, name);
	const DWORD style{ WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN };
	if (maximized) {
		const auto [x, y, w, h] = calculate_maximized_window_rectangle();
		return CreateWindow(type.data(), name.data(), style, x, y, w, h, nullptr, nullptr, windows::current_instance(), nullptr);
	} else {
		const int x{ CW_USEDEFAULT };
		const int y{ CW_USEDEFAULT };
		const auto [w, h] = calculate_actual_window_size(width, height, style);
		return CreateWindow(type.data(), name.data(), style, x, y, w, h, nullptr, nullptr, windows::current_instance(), nullptr);
	}
}

windows_window::windows_window(std::string_view title, std::optional<vector2i> size) {
	const bool maximized{ !size.has_value() };
	window_handle = create_window(title, "main", size.value_or(0).x, size.value_or(0).y, maximized);
	if (window_handle) {
		device_context = GetDC(window_handle);
		set_data();
		show(maximized);
	} else {
		error("draw", "Failed to create window");
	}
}

windows_window::~windows_window() {
	destroy_window(window_handle, device_context);
}

std::shared_ptr<render_context> windows_window::create_compatibility_render_context() {
	if (const auto default_window_handle = create_window("Compatibility", "compatibility", 0, 0, false)) {
		const auto default_device_context = GetDC(default_window_handle);
		auto compatibility_context = std::make_shared<wgl_compatibility_context>(default_device_context);
		destroy_window(default_window_handle, default_device_context);
		return compatibility_context;
	} else {
		error("draw", "Failed to create compatibility window");
		return nullptr;
	}
}

std::shared_ptr<render_context> windows_window::create_render_context(std::optional<int> samples) const {
	return std::make_shared<wgl_attribute_context>(device_context, samples);
}

void windows_window::show(bool maximized) {
	int show_command{ windows::show_command() };
	if (maximized && (show_command == SW_SHOWDEFAULT || show_command == SW_SHOWNORMAL)) {
		show_command = SW_MAXIMIZE;
	}
	ShowWindow(window_handle, show_command);
}

void windows_window::poll() {
	MSG message{};
	while (PeekMessage(&message, window_handle, 0, 0, PM_REMOVE)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

void windows_window::set_data() {
	SetWindowLongPtr(window_handle, 0, reinterpret_cast<LONG_PTR>(this));
}

bool windows_window::is_open() const {
	return window_handle != nullptr;
}

vector2i windows_window::position() const {
	RECT rectangle;
	GetWindowRect(window_handle, &rectangle);
	return { static_cast<int>(rectangle.left), static_cast<int>(rectangle.top) };
}

vector2i windows_window::size() const {
	RECT rectangle;
	GetClientRect(window_handle, &rectangle);
	return { static_cast<int>(rectangle.right), static_cast<int>(rectangle.bottom) };
}

std::string windows_window::title() const {
	// int size = GetWindowTextLength(windowHandle);
	CHAR buffer[128]{};
	GetWindowText(window_handle, buffer, 127);
	return buffer;
}

void windows_window::set_size(vector2i size) {
	RECT area{ 0, 0, size.x, size.y };
	const auto style = GetWindowLong(window_handle, GWL_STYLE);
	AdjustWindowRect(&area, style, false);
	area.right -= area.left;
	area.bottom -= area.top;
	SetWindowPos(window_handle, nullptr, 0, 0, area.right, area.bottom, SWP_NOMOVE | SWP_NOZORDER);
}

void windows_window::set_display_mode(display_mode mode) {
	// todo: change mode
	switch (mode) {
	case display_mode::windowed:
		break;
	case display_mode::fullscreen:
		break;
	case display_mode::fullscreen_desktop:
		break;
	default:
		warning("bugs", "Window mode not found: {}", mode);
		return;
	}
	last_set_display_mode = mode;
}

window::display_mode windows_window::current_display_mode() const {
	return last_set_display_mode;
}

void windows_window::maximize() {
	error("bugs", "Not implemented.");
	ASSERT(false);
}

void windows_window::set_title(std::string_view title) {
	SetWindowText(window_handle, title.data());
}

void windows_window::set_icon_from_resource(int resource_id) {
	if (const HICON icon{ LoadIcon(platform::windows::current_instance(), MAKEINTRESOURCE(resource_id)) }) {
		SendMessage(window_handle, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
		SendMessage(window_handle, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
		// no need to destroy icon, since it's shared
	} else {
		warning("draw", "Failed to load icon resource {}", resource_id);
	}
}

void windows_window::swap() {
	if (!SwapBuffers(device_context)) {
		const auto message = platform::windows::get_error_message(GetLastError());
		warning("draw", "Error: {}\nHDC: {}\nHWND: {}", message, (void*)device_context, (void*)window_handle);
	}
}

HWND windows_window::handle() const {
	return window_handle;
}

HDC windows_window::get_device_context() const {
	return device_context;
}

}
