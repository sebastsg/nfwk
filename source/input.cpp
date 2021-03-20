#include "input.hpp"

#include "graphics/window.hpp"

#include <Windows.h>
#include "graphics/windows_window.hpp"

namespace nfwk {

mouse::mouse(window* parent_window) : parent_window{ parent_window } {

}

int mouse::x() const {
	return position().x;
}

int mouse::y() const {
	return position().y;
}

vector2i mouse::position() const {
	POINT cursor;
	GetCursorPos(&cursor);
	if (const auto* platform_window = dynamic_cast<const platform::windows_window*>(parent_window)) {
		ScreenToClient(platform_window->handle(), &cursor);
	}
	return { cursor.x, cursor.y };
}

bool mouse::is_button_down(button button) const {
	switch (button) {
	case button::left: return (GetAsyncKeyState(VK_LBUTTON) & 0b1000000000000000) == 0b1000000000000000;
	case button::middle: return (GetAsyncKeyState(VK_MBUTTON) & 0b1000000000000000) == 0b1000000000000000;
	case button::right: return (GetAsyncKeyState(VK_RBUTTON) & 0b1000000000000000) == 0b1000000000000000;
	default: return false;
	}
}

keyboard::keyboard() {
	press_key = press.listen([this](key pressed_key) {
		keys[static_cast<std::size_t>(pressed_key)] = true;
	});
	release_key = release.listen([this](key released_key) {
		keys[static_cast<std::size_t>(released_key)] = false;
	});
}

bool keyboard::is_key_down(key check_key) const {
	return keys[static_cast<std::size_t>(check_key)];
}

}

std::ostream& operator<<(std::ostream& out, nfwk::mouse::button button) {
	switch (button) {
	case nfwk::mouse::button::none: return out << "None";
	case nfwk::mouse::button::left: return out << "Left";
	case nfwk::mouse::button::middle: return out << "Middle";
	case nfwk::mouse::button::right: return out << "Right";
	default: return out << "Unknown";
	}
}

std::ostream& operator<<(std::ostream& out, nfwk::key key) {
	switch (key) {
	case nfwk::key::backspace: return out << "Backspace";
	case nfwk::key::tab: return out << "Tab";
	case nfwk::key::enter: return out << "Enter";
	case nfwk::key::pause: return out << "Pause";
	case nfwk::key::caps_lock: return out << "Caps Lock";
	case nfwk::key::escape: return out << "Escape";
	case nfwk::key::space: return out << "Space";
	case nfwk::key::left: return out << "Left";
	case nfwk::key::up: return out << "Up";
	case nfwk::key::right: return out << "Right";
	case nfwk::key::down: return out << "Down";
	case nfwk::key::print_screen: return out << "Print Screen";
	case nfwk::key::del: return out << "Delete";
	case nfwk::key::num_0:
	case nfwk::key::num_1:
	case nfwk::key::num_2:
	case nfwk::key::num_3:
	case nfwk::key::num_4:
	case nfwk::key::num_5:
	case nfwk::key::num_6:
	case nfwk::key::num_7:
	case nfwk::key::num_8:
	case nfwk::key::num_9: return out << static_cast<char>(key);
	case nfwk::key::a:
	case nfwk::key::b:
	case nfwk::key::c:
	case nfwk::key::d:
	case nfwk::key::e:
	case nfwk::key::f:
	case nfwk::key::g:
	case nfwk::key::h:
	case nfwk::key::i:
	case nfwk::key::j:
	case nfwk::key::k:
	case nfwk::key::l:
	case nfwk::key::m:
	case nfwk::key::n:
	case nfwk::key::o:
	case nfwk::key::p:
	case nfwk::key::q:
	case nfwk::key::r:
	case nfwk::key::s:
	case nfwk::key::t:
	case nfwk::key::u:
	case nfwk::key::v:
	case nfwk::key::w:
	case nfwk::key::x:
	case nfwk::key::y:
	case nfwk::key::z: return out << static_cast<char>(key);
	case nfwk::key::f1:
	case nfwk::key::f2:
	case nfwk::key::f3:
	case nfwk::key::f4:
	case nfwk::key::f5:
	case nfwk::key::f6:
	case nfwk::key::f7:
	case nfwk::key::f8:
	case nfwk::key::f9:
	case nfwk::key::f10:
	case nfwk::key::f11:
	case nfwk::key::f12: return out << 'F' << static_cast<char>(key) - static_cast<char>(nfwk::key::f1) + 1;
	case nfwk::key::num_lock: return out << "Num Lock";
	case nfwk::key::scroll_lock: return out << "Scroll Lock";
	case nfwk::key::left_shift: return out << "Left Shift";
	case nfwk::key::right_shift: return out << "Right Shift";
	case nfwk::key::left_control: return out << "Left Control";
	case nfwk::key::right_control: return out << "Right Control";
	default: return out << "Unknown (" << static_cast<char>(key) << ")";
	}
}
