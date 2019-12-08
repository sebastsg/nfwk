#pragma once

#include "platform.hpp"

#if PLATFORM_WINDOWS && ENABLE_WINDOW

#include "windows_platform.hpp"
#include "math.hpp"
#include "input.hpp"

#if ENABLE_GL
#include "windows_gl.hpp"
#endif

namespace no {

class window;

namespace platform {

class windows_window {
public:

	windows_window(window& window, std::string_view title, int width, int height, int samples);
	windows_window(window& window, std::string_view title, int width, int height);
	windows_window(window& window, std::string_view title, int samples);
	windows_window(window& window, std::string_view title);

	windows_window(const windows_window&) = delete;
	windows_window(windows_window&&) = delete;

	~windows_window();

	windows_window& operator=(const windows_window&) = delete;
	windows_window& operator=(windows_window&&) = delete;

	void poll();
	void set_base_window(window& window);

	bool is_open() const;
	vector2i position() const;
	vector2i size() const;

	std::string title() const;

	void set_size(const vector2i& size);
	void set_title(std::string_view title);
	void set_icon_from_resource(int resource_id);
	void set_cursor(mouse::cursor icon);
	void set_viewport(int x, int y, int width, int height);
	void set_scissor(int x, int y, int width, int height);
	void set_clear_color(const vector3f& color);
	bool set_swap_interval(swap_interval interval);

	void clear();
	void swap();

	HWND handle() const;

private:

	void create_default_window(window& window, std::string_view title, int width, int height, bool maximized);
	void create_arb_window(window& window, std::string_view title, int width, int height, int samples, bool maximized);
	void show(bool maximized);

	HWND window_handle{ nullptr };
	HDC device_context{ nullptr };
	window* base_window{ nullptr };
	platform_render_context render_context;

};

}

}

#endif
