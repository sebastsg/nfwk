module;

#include <Windows.h>

export module nfwk.input:mouse;

import std.core;
import nfwk.core;

export namespace nfwk {

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

	mouse() = default;

	int x() const {
		return position().x;
	}

	int y() const {
		return position().y;
	}

	vector2i position() const {
		POINT cursor;
		GetCursorPos(&cursor);
		//ScreenToClient(parent_window->handle(), &cursor);
		return { cursor.x, cursor.y };
	}

	bool is_button_down(button button) const {
		switch (button) {
		case button::left: return (GetAsyncKeyState(VK_LBUTTON) & 0b1000000000000000) == 0b1000000000000000;
		case button::middle: return (GetAsyncKeyState(VK_MBUTTON) & 0b1000000000000000) == 0b1000000000000000;
		case button::right: return (GetAsyncKeyState(VK_RBUTTON) & 0b1000000000000000) == 0b1000000000000000;
		default: return false;
		}
	}

};

}

#if 0
export std::ostream& operator<<(std::ostream& out, nfwk::mouse::button button) {
	switch (button) {
	case nfwk::mouse::button::none: return out << "None";
	case nfwk::mouse::button::left: return out << "Left";
	case nfwk::mouse::button::middle: return out << "Middle";
	case nfwk::mouse::button::right: return out << "Right";
	default: return out << "Unknown";
	}
}
#endif
