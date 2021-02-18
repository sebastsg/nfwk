module;

#include <Windows.h>

export module nfwk.input:keyboard;

import std.core;
import nfwk.core;

export namespace nfwk {

enum class key {

	unknown = 0,

	backspace = 8,
	tab = 9,
	enter = 13,
	pause = 19,
	caps_lock = 20,
	escape = 27,
	space = 32,
	page_up = 33,
	page_down = 34,
	end = 35,
	home = 36,
	left = 37,
	up = 38,
	right = 39,
	down = 40,
	print_screen = 44,
	insert = 45,
	del = 46,

	num_0 = 48, // '0'
	num_1 = 49,
	num_2 = 50,
	num_3 = 51,
	num_4 = 52,
	num_5 = 53,
	num_6 = 54,
	num_7 = 55,
	num_8 = 56,
	num_9 = 57,

	a = 65, // 'A'
	b = 66,
	c = 67,
	d = 68,
	e = 69,
	f = 70,
	g = 71,
	h = 72,
	i = 73,
	j = 74,
	k = 75,
	l = 76,
	m = 77,
	n = 78,
	o = 79,
	p = 80,
	q = 81,
	r = 82,
	s = 83,
	t = 84,
	u = 85,
	v = 86,
	w = 87,
	x = 88,
	y = 89,
	z = 90,

	f1 = 112,
	f2 = 113,
	f3 = 114,
	f4 = 115,
	f5 = 116,
	f6 = 117,
	f7 = 118,
	f8 = 119,
	f9 = 120,
	f10 = 121,
	f11 = 122,
	f12 = 123,

	num_lock = 144,
	scroll_lock = 145,

	left_shift = 160,
	right_shift = 161,

	left_control = 162,
	right_control = 163,

	plus = 187,
	minus = 189,

	max_keys,

};

class keyboard {
public:

	event<key> press;
	event<key> repeated_press;
	event<key> release;
	event<unsigned int> input;

	keyboard() {
		std::fill(std::begin(keys), std::end(keys), false);
		press_key = press.listen([this](key pressed_key) {
			keys[static_cast<std::size_t>(pressed_key)] = true;
		});
		release_key = release.listen([this](key released_key) {
			keys[static_cast<std::size_t>(released_key)] = false;
		});
	}

	bool is_key_down(key check_key) const {
		return keys[static_cast<std::size_t>(check_key)];
	}

private:

	bool keys[static_cast<std::size_t>(key::max_keys)];
	event_listener press_key;
	event_listener release_key;

};

}

#if 0
export std::ostream& operator<<(std::ostream& out, nfwk::key key) {
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
#endif
