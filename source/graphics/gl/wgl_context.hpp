#pragma once

#include "platform.hpp"

#include "windows_platform.hpp"
#include "graphics/draw.hpp"
#include "graphics/window.hpp"

#include <optional>

#ifndef _WINDEF_
struct HGLRC__;
typedef HGLRC__* HGLRC;
#endif

namespace nfwk::platform {

class wgl_context : public render_context {
public:

	static HGLRC current_gl_context_handle();
	static HDC current_device_context_handle();

	~wgl_context() override;

	bool exists() const override;
	bool is_current() const override;
	void log_info() const override;

	void set_viewport(int x, int y, int width, int height) override;
	void set_scissor(int x, int y, int width, int height) override;
	void set_clear_color(const vector3f& color) override;
	bool set_swap_interval(swap_interval interval) override;
	void make_current(const window& window) override;
	void clear() override;

	HGLRC handle() const;

protected:

	void initialize_glew();
	void initialize_gl();
	bool set_swap_interval(int interval);
	void make_current(HDC device_context);

	HGLRC gl_context{ nullptr };

};

class wgl_compatibility_context : public wgl_context {
public:

	wgl_compatibility_context(HDC device_context);

private:

	void set_pixel_format(HDC device_context);

};

class wgl_attribute_context : public wgl_context {
public:

	wgl_attribute_context(HDC device_context, std::optional<int> samples);

private:

	void set_pixel_format(HDC device_context);
	void enable_multisampling();

	int samples{ 0 };

};

}
