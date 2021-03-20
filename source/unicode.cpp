#include "unicode.hpp"

namespace nfwk::utf8 {

std::string from_latin1(std::string_view latin1_string) {
	std::string utf8_string;
	for (auto& latin1_char : latin1_string) {
		if (static_cast<std::uint8_t>(latin1_char) < 0b10000000) {
			utf8_string.push_back(latin1_char);
		} else {
			utf8_string.push_back(0b11000000 | (static_cast<std::uint8_t>(latin1_char) >> 6));
			utf8_string.push_back(0b10000000 | (static_cast<std::uint8_t>(latin1_char) & 0b00111111));
		}
	}
	return utf8_string;
}

std::uint32_t next_character(std::string_view utf8_string, std::size_t* index) {
	if (*index >= utf8_string.size()) {
		return unicode::replacement_character;
	}
	std::uint8_t current{ static_cast<std::uint8_t>(utf8_string[*index]) };
	(*index)++;
	if (current < 0b10000000) {
		return static_cast<std::uint32_t>(current);
	}
	std::uint32_t character{ unicode::replacement_character };
	int remaining{ 0 };
	if (current >= 0b11111100) {
		if ((current & 0b11111110) == 0b11111100) {
			character = static_cast<std::uint32_t>(current & 0b00000001);
			remaining = 5;
		}
	} else if (current >= 0b11111000) {
		if ((current & 0b11111100) == 0b11111000) {
			character = static_cast<std::uint32_t>(current & 0b00000011);
			remaining = 4;
		}
	} else if (current >= 0b11110000) {
		if ((current & 0b11111000) == 0b11110000) {
			character = static_cast<std::uint32_t>(current & 0b00000111);
			remaining = 3;
		}
	} else if (current >= 0b11100000) {
		if ((current & 0b11110000) == 0b11100000) {
			character = static_cast<std::uint32_t>(current & 0b00001111);
			remaining = 2;
		}
	} else if (current >= 0b11000000) {
		if ((current & 0b11100000) == 0b11000000) {
			character = static_cast<std::uint32_t>(current & 0b00011111);
			remaining = 1;
		}
	}
	while (remaining-- > 0 && utf8_string.size() > *index) {
		current = utf8_string[*index];
		if ((current & 0b11000000) != 0b10000000) {
			return unicode::replacement_character;
		}
		character <<= 6;
		character |= (current & 0b00111111);
		(*index)++;
	}
	return character;
}

}
