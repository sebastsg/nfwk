export module nfwk.core:platform.base;

import std.core;

export namespace nfwk {

enum class platform_type { windows, linux, unknown };

enum class system_cursor {
	none,
	arrow,
	beam,
	resize_all,
	resize_horizontal,
	resize_vertical,
	resize_diagonal_from_bottom_left,
	resize_diagonal_from_top_left,
	block,
	hand,
	help,
	cross,
	wait
};

}

#if 0
export std::ostream& operator<<(std::ostream& out, nfwk::system_cursor cursor) {
	switch (cursor) {
	case nfwk::system_cursor::none: return out << "None";
	case nfwk::system_cursor::arrow: return out << "Arrow";
	case nfwk::system_cursor::beam: return out << "Beam";
	case nfwk::system_cursor::resize_all: return out << "Resize (all)";
	case nfwk::system_cursor::resize_horizontal: return out << "Resize (horizontal)";
	case nfwk::system_cursor::resize_vertical: return out << "Resize (vertical)";
	case nfwk::system_cursor::resize_diagonal_from_bottom_left: return out << "Resize (bottom left -> top right)";
	case nfwk::system_cursor::resize_diagonal_from_top_left: return out << "Resize (top left -> bottom right)";
	case nfwk::system_cursor::block: return out << "Block";
	case nfwk::system_cursor::hand: return out << "Hand";
	case nfwk::system_cursor::help: return out << "Help";
	case nfwk::system_cursor::cross: return out << "Cross";
	case nfwk::system_cursor::wait: return out << "Wait";
	default: return out << "Invalid";
	}
}
#endif
