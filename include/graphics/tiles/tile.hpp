#pragma once

namespace no::tiles {

class tile {
public:

	union {
		int value{};
		unsigned char corner[4];
		struct {
			unsigned char top_left;
			unsigned char top_right;
			unsigned char bottom_left;
			unsigned char bottom_right;
		};
	};

	tile(unsigned char top_left, unsigned char top_right, unsigned char bottom_left, unsigned char bottom_right);
	tile(unsigned char type);
	tile() = default;

	bool is_only(unsigned char type) const;

};

}
