#pragma once

#include "platform.hpp"

#if PLATFORM_WINDOWS && ENABLE_GL

#include "windows_platform.hpp"
#include "graphics/draw.hpp"

#ifndef _WINDEF_
struct HGLRC__;
typedef HGLRC__* HGLRC;
#endif

namespace no::platform {

class windows_gl_context {
public:

	static HGLRC current_context_handle();

	void create_default(HDC device_context_handle);
	void create_with_attributes(HDC device_context_handle, int samples);
	HGLRC handle() const;
	bool exists() const;
	bool is_current() const;
	void make_current();
	void set_viewport(int x, int y, int width, int height);
	void set_scissor(int x, int y, int w, int h);
	void set_clear_color(const vector3f& color);
	void clear();
	void destroy();

	void log_renderer_info() const;

	void enable_multisampling();

	void initialize_glew();
	void initialize_gl();

	bool set_swap_interval(swap_interval interval);

private:

	bool set_swap_interval(int interval);

	HGLRC gl_context = nullptr;
	HDC device_context = nullptr;
	bool is_arb_context = false;

};

}

#endif
