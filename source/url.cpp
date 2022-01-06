#include "url.hpp"

#include <optional>

namespace nfwk {

bool read_hex_digit(unsigned char* destination, char value) {
	const bool lower = (value >= 'a' && value <= 'f');
	const bool upper = (value >= 'A' && value <= 'F');
	const bool digit = (value >= '0' && value <= '9');
	*destination = static_cast<unsigned char>(value - (digit ? '0' : (upper ? 'A' : 'a') - 10));
	return lower || upper || digit;
}

std::optional<unsigned char> read_hex_byte(char in_a, char in_b) {
	unsigned char a, b;
	const bool first = read_hex_digit(&a, in_a);
	const bool second = read_hex_digit(&b, in_b);
	if (first && second) {
		return 16 * a + b;
	} else {
		return std::nullopt;
	}
}

std::string decode_url(std::string_view url) {
	auto it = url.begin();
	auto end = url.end();
	std::string destination;
	destination.reserve(url.size() * 2);
	while (end > it) {
		if (*it != '%') {
			destination.push_back(*it);
			it++;
			continue;
		}
		if (it + 3 > end) {
			break;
		}
		++it;
		if (const auto byte = read_hex_byte(*it, *(it + 1))) {
			destination.push_back(byte.value());
		}
		++it;
		++it;
	}
	return destination;
}

#if 0
void ucs2_decode(unsigned int value, char** buffer) {
	if (value < 0x80) {
		*(*buffer)++ = value;
	} else if (value < 0x800) {
		*(*buffer)++ = (value >> 6) | 0xc0;
		*(*buffer)++ = (value & 0x3f) | 0x80;
	} else if (value < 0xffff) {
		*(*buffer)++ = (value >> 12) | 0xe0;
		*(*buffer)++ = ((value >> 6) & 0x3f) | 0x80;
		*(*buffer)++ = (value & 0x3f) | 0x80;
	} else if (value <= 0x1fffff) {
		*(*buffer)++ = 0xf0 | (value >> 18);
		*(*buffer)++ = 0x80 | ((value >> 12) & 0x3f);
		*(*buffer)++ = 0x80 | ((value >> 6) & 0x3f);
		*(*buffer)++ = 0x80 | (value & 0x3f);
	}
}

unsigned short read_hex_short(char** data) {
	unsigned char a, b, c, d;
	const bool first = read_hex_digit(&a, *(*data)++);
	const bool second = read_hex_digit(&b, *(*data)++);
	const bool third = read_hex_digit(&c, *(*data)++);
	const bool fourth = read_hex_digit(&d, *(*data)++);
	if (first && second && third && fourth) {
		a <<= 4;
		c <<= 4;
		a &= 0b11110000;
		b &= 0b00001111;
		c &= 0b11110000;
		d &= 0b00001111;
		unsigned short value{ 0 };
		value = a | b;
		value <<= 8;
		value |= c | d;
		return value;
	}
	return 0;
}

char* json_decode_string(char* string) {
	char* begin = string;
	char* end = string + strlen(string);
	char* dest = string;
	while (end > string) {
		if (*string == '\\' && *(string + 1) == 'u') {
			if (string + 5 > end) {
				break;
			}
			string += 2;
			unsigned short value = read_hex_short(&string);
			ucs2_decode(value, &dest);
		} else {
			*(dest++) = *(string++);
		}
	}
	*dest = '\0';
	return begin;
}
#endif

}
