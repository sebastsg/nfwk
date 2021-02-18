export module nfwk.tiles:tile;

export namespace nfwk::tiles {

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

	tile(unsigned char top_left, unsigned char top_right, unsigned char bottom_left, unsigned char bottom_right)
		: top_left{ top_left }, top_right{ top_right }, bottom_left{ bottom_left }, bottom_right{ bottom_right } {
	}

	tile(unsigned char type)
		: top_left{ type }, top_right{ type }, bottom_left{ type }, bottom_right{ type } {
	}

	tile() = default;

	bool is_only(unsigned char type) const {
		return top_left == type && top_right == type && bottom_left == type && bottom_right == type;
	}

};

}
