#include "wgl_context.hpp"
#include "graphics/windows_window.hpp"
#include "assert.hpp"

#include <glew/glew.h>
#include <glew/wglew.h>

#include "log.hpp"

namespace nfwk::platform {

HGLRC wgl_context::current_gl_context_handle() {
	return wglGetCurrentContext();
}

HDC wgl_context::current_device_context_handle() {
	return wglGetCurrentDC();
}

wgl_context::~wgl_context() {
	if (exists()) {
		// todo: this code must be improved/fixed, as it's assuming the current device context can be used.
		if (is_current()) {
			info(draw::log, u8"Calling delete event");
			on_delete.emit();
			wglMakeCurrent(nullptr, nullptr);
		} else if (on_delete.total_listeners() > 0) {
			bug(u8"{} listeners for delete event, and this context is not current. If >0, expect crash.", on_delete.total_listeners());
		}
		info(draw::log, u8"Deleting context");
		wglDeleteContext(gl_context);
	}
}

void wgl_context::initialize_glew() {
	if (const auto glew_error = glewInit(); glew_error != GLEW_OK) {
		error(draw::log, u8"Failed to initialize GLEW");
		ASSERT(glew_error == GLEW_OK);
	}
}

void wgl_context::initialize_gl() {
	message(draw::log, u8"Enabling depth testing");
	CHECK_GL_ERROR(glEnable(GL_DEPTH_TEST));
	message(draw::log, u8"Setting depth function: \"less than or equal\"");
	CHECK_GL_ERROR(glDepthFunc(GL_LEQUAL));
	message(draw::log, u8"Enabling blending");
	CHECK_GL_ERROR(glEnable(GL_BLEND));
	// https://www.opengl.org/sdk/docs/man/html/glBlendFunc.xhtml
	message(draw::log, u8"Setting blend function. Source: \"Source alpha\". Destination: \"One minus source alpha\"");
	CHECK_GL_ERROR(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	message(draw::log, u8"Enabling scissor testing");
	CHECK_GL_ERROR(glEnable(GL_SCISSOR_TEST));
}

HGLRC wgl_context::handle() const {
	return gl_context;
}

bool wgl_context::exists() const {
	return gl_context != nullptr;
}

bool wgl_context::is_current() const {
	return current_gl_context_handle() == gl_context;
}

void wgl_context::make_current(const window& window) {
	if (const auto* platform_window = dynamic_cast<const platform::windows_window*>(&window)) {
		make_current(platform_window->get_device_context());
		const auto [width, height] = window.size();
		set_viewport(0, 0, width, height);
	} else {
		bug(u8"Unknown window implementation");
	}
}

void wgl_context::make_current(HDC device_context) {
	current_context = this;
	wglMakeCurrent(device_context, gl_context);
	initialize_glew();
}

void wgl_context::set_viewport(int x, int y, int width, int height) {
	CHECK_GL_ERROR(glViewport(x, y, width, height));
}

void wgl_context::set_scissor(int x, int y, int width, int height) {
	CHECK_GL_ERROR(glScissor(x, y, width, height));
}

void wgl_context::set_clear_color(const vector3f& color) {
	CHECK_GL_ERROR(glClearColor(color.x, color.y, color.z, 1.0f));
}

void wgl_context::clear() {
	CHECK_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void wgl_context::log_info() const {
	info(draw::log, u8"[b]Windows OpenGL[/b]"
		"\n[b]Version:[/b] {}"
		"\n[b]Vendor:[/b] {}"
		"\n[b]Renderer:[/b] {}"
		"\n[b]Shading Language Version:[/b] {}",
		glGetString(GL_VERSION),
		glGetString(GL_VENDOR),
		glGetString(GL_RENDERER),
		glGetString(GL_SHADING_LANGUAGE_VERSION)
	);
}

bool wgl_context::set_swap_interval(swap_interval interval) {
	return [&] {
		switch (interval) {
		case swap_interval::late: return set_swap_interval(-1);
		case swap_interval::immediate: return set_swap_interval(0);
		case swap_interval::sync: return set_swap_interval(1);
		default: return false;
		}
	}();
}

bool wgl_context::set_swap_interval(int interval) {
	const auto status = wglSwapIntervalEXT(interval);
	if (status) {
		message(draw::log, u8"Set swap interval to {}", interval);
	} else {
		warning(draw::log, u8"Failed to set swap interval to {}. Error: {}", interval, GetLastError());
	}
	return status;
}

wgl_compatibility_context::wgl_compatibility_context(HDC device_context) {
	set_pixel_format(device_context);
	message(draw::log, u8"Creating default context");
	gl_context = wglCreateContext(device_context);
	if (!gl_context) {
		error(draw::log, u8"Failed to create default context");
	}
	make_current(device_context);
	initialize_gl();
}

void wgl_compatibility_context::set_pixel_format(HDC device_context) {
	message(draw::log, u8"Setting pixel format (compatibility)");
	PIXELFORMATDESCRIPTOR descriptor{};
	descriptor.nSize = sizeof(descriptor);
	descriptor.nVersion = 1;
	descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	descriptor.iLayerType = PFD_MAIN_PLANE;
	descriptor.iPixelType = PFD_TYPE_RGBA;
	descriptor.cColorBits = 32;
	const int format{ ChoosePixelFormat(device_context, &descriptor) };
	if (format == 0) {
		error(draw::log, u8"Did not find suitable pixel format");
		return;
	}
	DescribePixelFormat(device_context, format, sizeof(PIXELFORMATDESCRIPTOR), &descriptor);
	if (!SetPixelFormat(device_context, format, &descriptor)) {
		error(draw::log, u8"Failed to set pixel format");
	}
	info(draw::log, u8"Pixel Format: {}\nDouble buffer: {}", format, ((descriptor.dwFlags & PFD_DOUBLEBUFFER) ? "Yes" : "No"));
}

wgl_attribute_context::wgl_attribute_context(HDC device_context, std::optional<int> samples) : samples{ samples.value_or(0) } {
	set_pixel_format(device_context);
	const int attributes[]{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 6,
		0
	};
	message(draw::log, u8"Creating context with attributes");
	gl_context = wglCreateContextAttribsARB(device_context, nullptr, attributes);
	if (!gl_context) {
		warning(draw::log, u8"Failed to create context. OpenGL {}.{} not supported", attributes[1], attributes[3]);
		message(draw::log, u8"Creating fallback context");
		gl_context = wglCreateContext(device_context);
		if (!gl_context) {
			error(draw::log, u8"Failed to create fallback context");
			return;
		}
	}
	make_current(device_context);
	initialize_gl();
	if (samples > 0) {
		enable_multisampling();
	}
}

void wgl_attribute_context::set_pixel_format(HDC device_context) {
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
	message(draw::log, u8"Setting pixel format");
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
	const auto success = wglChoosePixelFormatARB(device_context, int_attributes, float_attributes, 1, &format, &count);
	if (!success || count == 0) {
		warning(draw::log, u8"Failed to find pixel format");
		return;
	}
	PIXELFORMATDESCRIPTOR descriptor{};
	DescribePixelFormat(device_context, format, sizeof(PIXELFORMATDESCRIPTOR), &descriptor);
	if (!SetPixelFormat(device_context, format, &descriptor)) {
		error(draw::log, u8"Failed to set pixel format");
	}
	info(draw::log, u8"Pixel Format: {}\nDouble buffer: {}\nSamples: {}", format, ((descriptor.dwFlags & PFD_DOUBLEBUFFER) ? "Yes" : "No"), samples);
}

void wgl_attribute_context::enable_multisampling() {
	if (samples > 0) {
		message(draw::log, u8"Enabled multisampling");
		CHECK_GL_ERROR(glEnable(GL_MULTISAMPLE));
	}
}

}
