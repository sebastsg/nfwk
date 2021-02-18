module;

#include <glew/glew.h>
#include <glew/wglew.h>

#include "gl_debug.hpp"

#ifndef _WINDEF_
struct HGLRC__;
typedef HGLRC__* HGLRC;
#endif

export module nfwk.draw:windows_gl;

import nfwk.core;
import nfwk.graphics;
import :window_base;

namespace nfwk {

void set_pixel_format(HDC device_context) {
	message("draw", "Setting pixel format");
	PIXELFORMATDESCRIPTOR descriptor{};
	descriptor.nSize = sizeof(descriptor);
	descriptor.nVersion = 1;
	descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	descriptor.iLayerType = PFD_MAIN_PLANE;
	descriptor.iPixelType = PFD_TYPE_RGBA;
	descriptor.cColorBits = 32;
	const int format{ ChoosePixelFormat(device_context, &descriptor) };
	if (format == 0) {
		error("draw", "Did not find suitable pixel format");
		return;
	}
	DescribePixelFormat(device_context, format, sizeof(PIXELFORMATDESCRIPTOR), &descriptor);
	if (!SetPixelFormat(device_context, format, &descriptor)) {
		error("draw", "Failed to set pixel format");
	}
	info("draw", "Pixel Format: {}\nDouble buffer: {}", format, (descriptor.dwFlags & PFD_DOUBLEBUFFER) ? "Yes" : "No");
}

void set_pixel_format_arb(HDC device_context, int samples) {
	switch (samples) {
	case 0:
	case 1:
	case 2:
	case 4:
	case 8:
	case 16:
		break;
	default:
		samples = 1;
		break;
	}
	message("draw", "Setting pixel format using the ARB extension");
	const int int_attributes[]{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB, samples > 0 ? 1 : 0,
		WGL_SAMPLES_ARB, samples,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 24,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_DEPTH_BITS_ARB, 16,
		WGL_STENCIL_BITS_ARB, 0,
		0
	};
	const float float_attributes[]{
		0
	};
	int format{ 0 };
	unsigned int count{ 0 };
	const BOOL success{ wglChoosePixelFormatARB(device_context, int_attributes, float_attributes, 1, &format, &count) };
	if (!success || count == 0) {
		warning("draw", "Failed to find pixel format");
		return;
	}
	PIXELFORMATDESCRIPTOR descriptor{};
	DescribePixelFormat(device_context, format, sizeof(PIXELFORMATDESCRIPTOR), &descriptor);
	if (!SetPixelFormat(device_context, format, &descriptor)) {
		error("draw", "Failed to set pixel format");
	}
	info("draw", "Pixel Format: {}\nDouble buffer: {}\nSamples: {}", format, (descriptor.dwFlags & PFD_DOUBLEBUFFER) ? "Yes" : "No", samples);
}

}

export namespace nfwk {

class windows_gl_context {
public:

	static HGLRC current_context_handle() {
		return wglGetCurrentContext();
	}

	void create_default(HDC device_context_handle) {
		this->device_context = device_context;
		set_pixel_format(device_context);
		message("draw", "Creating default context");
		gl_context = wglCreateContext(device_context);
		if (!gl_context) {
			error("draw", "Failed to create default context");
		}
	}

	void create_with_attributes(HDC device_context_handle, int samples) {
		this->device_context = device_context;
		set_pixel_format_arb(device_context, samples);
		const int attributes[]{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 6,
			0
		};
		message("draw", "Creating context with attributes");
		gl_context = wglCreateContextAttribsARB(device_context, nullptr, attributes);
		is_arb_context = (gl_context != nullptr);
		if (!is_arb_context) {
			warning("draw", "Failed to create context. OpenGL {}.{} not supported", attributes[1], attributes[3]);
			message("draw", "Creating fallback context");
			gl_context = wglCreateContext(device_context);
			if (!gl_context) {
				error("draw", "Failed to create fallback context");
				return;
			}
		}
	}

	HGLRC handle() const {
		return gl_context;
	}

	bool exists() const {
		return gl_context != nullptr;
	}

	bool is_current() const {
		return current_context_handle() == gl_context;
	}

	void make_current() {
		wglMakeCurrent(device_context, gl_context);
		initialize_glew();
	}

	void set_viewport(int x, int y, int width, int height) {
		CHECK_GL_ERROR(glViewport(x, y, width, height));
	}

	void set_scissor(int x, int y, int width, int height) {
		CHECK_GL_ERROR(glScissor(x, y, width, height));
	}

	void set_clear_color(const vector3f& color) {
		CHECK_GL_ERROR(glClearColor(color.x, color.y, color.z, 1.0f));
	}

	void clear() {
		// todo: can check is_current() and change context, but does that cause more harm than good?
		CHECK_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	void destroy() {
		if (exists()) {
			if (is_current()) {
				wglMakeCurrent(nullptr, nullptr);
			}
			wglDeleteContext(gl_context);
		}
	}

	void log_renderer_info() const {
		info("draw", "[b]Windows OpenGL[/b]\n"
			"[b]Version:[/b] {}\n"
			"[b]Vendor:[/b] {}\n"
			"[b]Renderer:[/b] {}\n"
			"[b]Shading Language Version:[/b] {}",
			glGetString(GL_VERSION), 
			glGetString(GL_VENDOR), 
			glGetString(GL_RENDERER), 
			glGetString(GL_SHADING_LANGUAGE_VERSION)
		);
	}

	void enable_multisampling() {
		if (is_arb_context) {
			message("draw", "Enabled multisampling");
			CHECK_GL_ERROR(glEnable(GL_MULTISAMPLE));
		} else {
			warning("draw", "Cannot enable multisampling on non-ARB OpenGL context.");
		}
	}

	void initialize_glew() {
		if (const auto error_code = glewInit(); error_code != GLEW_OK) {
			error("draw", "Failed to initialize GLEW");
			ASSERT(error_code == GLEW_OK);
		}
	}

	void initialize_gl() {
		message("draw", "Enabling depth testing");
		CHECK_GL_ERROR(glEnable(GL_DEPTH_TEST));
		message("draw", "Setting depth function: \"less than or equal\"");
		CHECK_GL_ERROR(glDepthFunc(GL_LEQUAL));
		message("draw", "Enabling blending");
		CHECK_GL_ERROR(glEnable(GL_BLEND));
		// https://www.opengl.org/sdk/docs/man/html/glBlendFunc.xhtml
		message("draw", "Setting blend function. Source: \"Source alpha\". Destination: \"One minus source alpha\"");
		CHECK_GL_ERROR(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		message("draw", "Enabling scissor testing");
		CHECK_GL_ERROR(glEnable(GL_SCISSOR_TEST));
	}

	bool set_swap_interval(swap_interval interval) {
		return [&] {
			switch (interval) {
			case swap_interval::late: return set_swap_interval(-1);
			case swap_interval::immediate: return set_swap_interval(0);
			case swap_interval::sync: return set_swap_interval(1);
			default: return false;
			}
		}();
	}

private:

	bool set_swap_interval(int interval) {
		const auto status = wglSwapIntervalEXT(interval);
		if (status) {
			message("draw", "Set swap interval to {}", interval);
		} else {
			warning("draw", "Failed to set swap interval to {}. Error: {}", interval, GetLastError());
		}
		return status;
	}

	HGLRC gl_context{ nullptr };
	HDC device_context{ nullptr };
	bool is_arb_context{ false };

};

}
