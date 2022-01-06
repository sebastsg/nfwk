#pragma once

#include "platform.hpp"
#include "event.hpp"
#include "input.hpp"
#include "vector3.hpp"

#include <optional>
#include <memory>

namespace nfwk {

class window;
class subprogram;

enum class swap_interval { late, immediate, sync };

class render_context {
public:

	static render_context* get_current_context();
	static window* get_current_window();

	event<> on_delete;

	render_context() = default;
	render_context(const render_context&) = delete;
	render_context(render_context&&) = delete;

	virtual ~render_context() = default;

	render_context& operator=(const render_context&) = delete;
	render_context& operator=(render_context&&) = delete;

	virtual void set_viewport(int x, int y, int width, int height) = 0;
	virtual void set_scissor(int x, int y, int width, int height) = 0;
	virtual void set_clear_color(const vector3f& color) = 0;
	virtual bool set_swap_interval(swap_interval interval) = 0;

	virtual void make_current(const window& window) = 0;
	virtual void clear() = 0;

	virtual bool exists() const = 0;
	virtual bool is_current() const = 0;
	virtual void log_info() const = 0;

protected:

	static thread_local render_context* current_context;
	static thread_local window* current_window;

};

class window {
public:

	enum class display_mode { windowed, fullscreen, fullscreen_desktop };

	event<int, int> on_resize;
	event<> on_draw_begin;
	event<> on_draw;
	event<> on_draw_end;
	event<> on_close;

	keyboard keyboard;
	mouse mouse;

	window();
	window(const window&) = delete;
	window(window&&) = delete;

	virtual ~window();

	window& operator=(const window&) = delete;
	window& operator=(window&&) = delete;

	void attach_subprogram(subprogram& subprogram);
	bool has_attached_subprogram(const subprogram& subprogram) const;

	virtual void poll() = 0;
	virtual void swap() = 0;
	virtual void maximize() = 0;

	virtual void set_title(std::string_view title) = 0;
	virtual void set_size(vector2i size) = 0;

	virtual std::string title() const = 0;
	virtual vector2i size() const = 0;
	virtual vector2i position() const = 0;
	virtual bool is_open() const = 0;

	virtual void set_display_mode(display_mode mode) = 0;
	virtual display_mode current_display_mode() const = 0;

	virtual std::shared_ptr<render_context> create_render_context(std::optional<int> samples) const = 0;

private:

	std::vector<subprogram*> attached_subprograms;
	event_listener close_event_listener;
	event_listener draw_event_listener;
	event_listener frame_event_listener;

};

}

std::ostream& operator<<(std::ostream& out, nfwk::window::display_mode mode);
std::ostream& operator<<(std::ostream& out, nfwk::swap_interval interval);
