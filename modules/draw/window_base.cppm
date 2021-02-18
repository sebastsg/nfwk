export module nfwk.draw:window_base;

import std.core;
import nfwk.core;
import nfwk.input;

export namespace nfwk {

enum class window_display_mode { windowed, fullscreen, fullscreen_desktop };
enum class swap_interval { late, immediate, sync };

class window_base {
public:

	event<> close;
	event<int, int> resize;

	keyboard keyboard;
	mouse mouse;

	window_base() = default;
	window_base(const window_base&) = delete;

	window_base(window_base&& that) noexcept {
		last_set_display_mode = that.last_set_display_mode;
		close = std::move(that.close);
		resize = std::move(that.resize);
	}

	window_base& operator=(const window_base&) = delete;
	window_base& operator=(window_base&&) = delete;

	virtual void poll() = 0;
	virtual void clear() = 0;
	virtual void swap() = 0;

	virtual void set_clear_color(const vector3f& color) = 0;
	virtual void set_title(std::string_view title) = 0;

	virtual void set_width(int width) = 0;
	virtual void set_height(int height) = 0;
	virtual void set_size(const vector2i& size) = 0;

	virtual std::string title() const = 0;

	void maximize() {
		// todo: maximize
		set_viewport(0, 0, width(), height());
	}

	void set_display_mode(window_display_mode mode) {
		// todo: change mode
		switch (mode) {
		case window_display_mode::windowed:
		case window_display_mode::fullscreen:
		case window_display_mode::fullscreen_desktop:
			break;
		default:
			warning("graphics", "Window mode not found: {}", mode);
			return;
		}
		last_set_display_mode = mode;
	}

	virtual int width() const = 0;
	virtual int height() const = 0;
	virtual vector2i size() const = 0;

	virtual int x() const = 0;
	virtual int y() const = 0;
	virtual vector2i position() const = 0;

	virtual bool is_open() const = 0;

	virtual bool set_swap_interval(swap_interval interval) = 0;
	virtual void set_viewport(int x, int y, int width, int height) = 0;
	virtual void set_scissor(int x, int y, int width, int height) = 0;
	virtual void reset_scissor() = 0;

	window_display_mode current_display_mode() const {
		return last_set_display_mode;
	}

private:

	window_display_mode last_set_display_mode{ window_display_mode::windowed };

};

}

#if 0
export std::ostream& operator<<(std::ostream& out, nfwk::window_display_mode mode) {
	switch (mode) {
	case nfwk::window_display_mode::windowed: return out << "Windowed";
	case nfwk::window_display_mode::fullscreen: return out << "Fullscreen";
	case nfwk::window_display_mode::fullscreen_desktop: return out << "Fullscreen Desktop";
	default: return out << "Unknown";
	}
}
#endif
