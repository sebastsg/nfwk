#include "windows_window.hpp"

#if PLATFORM_WINDOWS && ENABLE_GRAPHICS

#include <Windows.h>
#include <windowsx.h>

#include "graphics/window.hpp"
#include "input.hpp"
#include "vector4.hpp"
#include "debug.hpp"

static no::vector2i current_mouse_position;

LRESULT WINAPI process_window_messages(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) {
	auto active_window = reinterpret_cast<no::window*>(GetWindowLongPtr(window_handle, 0));
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
		mouse.move.emit(no::vector2i{ relative_x, relative_y }, no::vector2i{ new_x, new_y });
		return 0;
	}
	case WM_LBUTTONDOWN:
		mouse.press.emit(no::mouse::button::left);
		return 0;
	case WM_LBUTTONUP:
		mouse.release.emit(no::mouse::button::left);
		return 0;
	case WM_LBUTTONDBLCLK:
		mouse.double_click.emit(no::mouse::button::left);
		return 0;
	case WM_RBUTTONDOWN:
		mouse.press.emit(no::mouse::button::right);
		return 0;
	case WM_RBUTTONUP:
		mouse.release.emit(no::mouse::button::right);
		return 0;
	case WM_RBUTTONDBLCLK:
		mouse.double_click.emit(no::mouse::button::right);
		return 0;
	case WM_MBUTTONDOWN:
		mouse.press.emit(no::mouse::button::middle);
		return 0;
	case WM_MBUTTONUP:
		mouse.release.emit(no::mouse::button::middle);
		return 0;
	case WM_MBUTTONDBLCLK:
		mouse.double_click.emit(no::mouse::button::middle);
		return 0;
	case WM_MOUSEWHEEL:
		// TODO: Maybe this won't work for mouse wheels without notches.
		mouse.scroll.emit(GET_WHEEL_DELTA_WPARAM(w_param) / WHEEL_DELTA);
		return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		const LPARAM repeat{ l_param >> 30 };
		no::key key{ static_cast<no::key>(w_param) };
		if (w_param == VK_SHIFT) {
			key = ((GetKeyState(VK_LSHIFT) & 0x8000) != 0 ? no::key::left_shift : no::key::right_shift);
		} else if (w_param == VK_CONTROL) {
			key = ((GetKeyState(VK_LCONTROL) & 0x8000) != 0 ? no::key::left_control : no::key::right_control);
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
		no::key key{ static_cast<no::key>(w_param) };
		if (w_param == VK_SHIFT) {
			if (keyboard.is_key_down(no::key::left_shift) && (GetKeyState(VK_LSHIFT) & 0x8000) == 0) {
				key = no::key::left_shift;
			} else {
				key = no::key::right_shift;
			}
		} else if (w_param == VK_CONTROL) {
			if (keyboard.is_key_down(no::key::left_control) && (GetKeyState(VK_LCONTROL) & 0x8000) == 0) {
				key = no::key::left_control;
			} else {
				key = no::key::right_control;
			}
		}
		keyboard.release.emit(key);
		return 0;
	}
	case WM_CHAR:
		keyboard.input.emit(static_cast<unsigned int>(w_param));
		return 0;
	case WM_SIZE:
		active_window->set_viewport(0, 0, LOWORD(l_param), HIWORD(l_param));
		active_window->resize.emit<int, int>(LOWORD(l_param), HIWORD(l_param));
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
		active_window->close.emit();
		return 0;
	default:
		return DefWindowProc(window_handle, message, w_param, l_param);
	}
}

namespace no {

namespace platform {

static bool create_window_class(WNDPROC procedure, std::string_view name) {
	MESSAGE_X("graphics", "Registering window class: " << name);
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
		CRITICAL_X("graphics", "Failed to register window class");
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

static HWND create_window(std::string_view name, std::string_view type, int width, int height, bool maximized) {
	MESSAGE_X("graphics", "Creating window: " << name << ". Class: " << type);
	const HINSTANCE instance{ windows::current_instance() };
	const DWORD style{ WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN };
	if (maximized) {
		const auto [x, y, w, h] = calculate_maximized_window_rectangle();
		return CreateWindow(type.data(), name.data(), style, x, y, w, h, nullptr, nullptr, instance, nullptr);
	} else {
		const int x{ CW_USEDEFAULT };
		const int y{ CW_USEDEFAULT };
		const auto [w, h] = calculate_actual_window_size(width, height, style);
		return CreateWindow(type.data(), name.data(), style, x, y, w, h, nullptr, nullptr, instance, nullptr);
	}
}

static void destroy_window(HWND window_handle, HDC device_context, windows_gl_context& context) {
	if (window_handle) {
		MESSAGE_X("graphics", "Destroying window");
		ReleaseDC(window_handle, device_context);
		context.destroy();
		DestroyWindow(window_handle);
	}
}

windows_window::windows_window(window& base_window, std::string_view title, int width, int height, int samples) {
	create_arb_window(base_window, title, width, height, samples, false);
}

windows_window::windows_window(window& base_window, std::string_view title, int width, int height) {
	create_default_window(base_window, title, width, height, false);
}

windows_window::windows_window(window& base_window, std::string_view title, int samples) {
	create_arb_window(base_window, title, 4, 4, samples, true);
}

windows_window::windows_window(window& base_window, std::string_view title) {
	create_default_window(base_window, title, 4, 4, true);
}

void windows_window::create_default_window(window& window, std::string_view title, int width, int height, bool maximized) {
	create_window_class(process_window_messages, "Main");
	window_handle = create_window(title, "Main", width, height, maximized);
	if (!window_handle) {
		CRITICAL_X("graphics", "Failed to create window");
		return;
	}
	device_context = GetDC(window_handle);
	render_context.create_default(device_context);
	render_context.make_current();
	render_context.initialize_gl();
	render_context.log_renderer_info();
	set_base_window(window);
	show(maximized);
}

void windows_window::create_arb_window(window& window, std::string_view title, int width, int height, int samples, bool maximized) {
	if (samples > 0) {
		create_window_class(DefWindowProc, "Default");
		const HWND default_window_handle{ create_window("Default", "Default", 4, 4, false) };
		if (!default_window_handle) {
			CRITICAL_X("graphics", "Failed to create default window");
			return;
		}
		const HDC default_device_context{ GetDC(default_window_handle) };
		render_context.create_default(default_device_context);
		render_context.make_current();
		destroy_window(default_window_handle, default_device_context, render_context);
	}
	create_window_class(process_window_messages, "Main");
	window_handle = create_window(title, "Main", width, height, maximized);
	if (!window_handle) {
		CRITICAL_X("graphics", "Failed to create window");
		return;
	}
	device_context = GetDC(window_handle);
	render_context.create_with_attributes(device_context, samples);
	render_context.make_current();
	render_context.initialize_gl();
	if (samples > 0) {
		render_context.enable_multisampling();
	}
	render_context.log_renderer_info();
	set_base_window(window);
	show(maximized);
}

void windows_window::show(bool maximized) {
	int show_command{ windows::show_command() };
	if ((show_command == SW_SHOWDEFAULT || show_command == SW_SHOWNORMAL) && maximized) {
		show_command = SW_MAXIMIZE;
	}
	if (!base_window) {
		WARNING_X("graphics", "Base window is not assigned. The message will not be processed.");
	}
	ShowWindow(window_handle, show_command);
}

windows_window::~windows_window() {
	destroy_window(window_handle, device_context, render_context);
}

void windows_window::poll() {
	MSG message;
	while (PeekMessage(&message, window_handle, 0, 0, PM_REMOVE)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

void windows_window::set_base_window(window& new_window) {
	base_window = &new_window;
	SetWindowLongPtr(window_handle, 0, reinterpret_cast<LONG_PTR>(base_window));
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

void windows_window::set_size(const vector2i& size) {
	RECT area{ 0, 0, size.x, size.y };
	const auto style = GetWindowLong(window_handle, GWL_STYLE);
	AdjustWindowRect(&area, style, false);
	area.right -= area.left;
	area.bottom -= area.top;
	SetWindowPos(window_handle, nullptr, 0, 0, area.right, area.bottom, SWP_NOMOVE | SWP_NOZORDER);
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
		WARNING_X("graphics", "Failed to load icon resource " << resource_id);
	}
}

void windows_window::set_viewport(int x, int y, int width, int height) {
	render_context.set_viewport(x, y, width, height);
}

void windows_window::set_scissor(int x, int y, int width, int height) {
	render_context.set_scissor(x, y, width, height);
}

void windows_window::set_clear_color(const vector3f& color) {
	render_context.set_clear_color(color);
}

bool windows_window::set_swap_interval(swap_interval interval) {
	return render_context.set_swap_interval(interval);
}

void windows_window::clear() {
	render_context.clear();
}

void windows_window::swap() {
	if (!SwapBuffers(device_context)) {
		WARNING_LIMIT_X("graphics", "HDC: " << device_context << "\nHWND: " << window_handle << "\nError: " << GetLastError(), 10);
	}
}

HWND windows_window::handle() const {
	return window_handle;
}

}

}

#endif
