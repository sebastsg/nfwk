#pragma once

#include "platform.hpp"

#include "event.hpp"
#include "vector2.hpp"

namespace nfwk {

class window;

class mouse {
public:

	enum class button { none, left, middle, right };

	event<vector2i, vector2i> move;
	event<button> press;
	event<button> release;
	event<button> double_click;
	event<int> scroll;
	event<bool> visibility;
	event<> icon;

	mouse(window* parent_window);
	mouse() = default;

	int x() const;
	int y() const;
	vector2i position() const;
	bool is_button_down(button button) const;

private:

	window* parent_window = nullptr;

};

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

	keyboard();

	bool is_key_down(key key) const;

private:

	bool keys[static_cast<std::size_t>(key::max_keys)]{};
	event_listener press_key;
	event_listener release_key;

};

}

std::ostream& operator<<(std::ostream& out, nfwk::mouse::button button);
std::ostream& operator<<(std::ostream& out, nfwk::key key);
