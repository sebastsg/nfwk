#pragma once

#include <string>
#include <cstdint>

namespace nfwk::unicode {
constexpr std::uint32_t replacement_character{ 0xfffd };
constexpr std::uint32_t byte_order_mark{ 0xfeff };
constexpr std::uint32_t byte_order_mark_swapped{ 0xfffe };
}

namespace nfwk::utf8 {
std::string from_latin1(std::string_view latin1_string);
std::uint32_t next_character(std::string_view utf8_string, std::size_t* index);
}
