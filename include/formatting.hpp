#pragma once

#include "utility_functions.hpp"

#include <optional>
#include <vector>
#include <format>

namespace nfwk::stdspec::formatter {

constexpr bool has_more_options(const auto& context) {
	return context.begin() != context.end() && *context.begin() != '}';
}

constexpr std::optional<std::string_view> read_options(auto& context) {
	std::size_t length{ 0 };
	const auto& begin = context.begin();
	while (has_more_options(context)) {
		length++;
		context.begin()++;
	}
	return length > 0 ? std::string_view{ &*begin, length } : std::optional<std::string_view>{};
}

template<typename Enum>
class enum_formatter {
public:

	constexpr auto parse(auto& context) {
		return context.begin();
	}

	auto format(const Enum& value, auto& context) {
		return format_to(context.out(), "{}", nfwk::enum_string(value));
	}

};

}

namespace std {

template<typename ElementType>
class formatter<std::vector<ElementType>> {
public:

	constexpr auto parse(auto& context) {
		if (const auto options = nfwk::stdspec::formatter::read_options(context)) {
			glue = options.value();
		}
		return context.begin();
	}

	auto format(const std::vector<ElementType>& values, auto& context) {
		if constexpr (std::is_same_v<ElementType, std::string_view> || std::is_same_v<ElementType, std::string>) {
			return format_to(context.out(), "{}", nfwk::merge_strings(values, glue));
		} else {
			return format_to(context.out(), "{}", nfwk::merge_strings(nfwk::to_strings(values), glue));
		}
	}

private:

	std::string_view glue{ ", " };

};

template<>
class formatter<std::filesystem::path> {
public:

	constexpr auto parse(auto& context) {
		if (const auto options = nfwk::stdspec::formatter::read_options(context)) {
			if (options == "q") {
				quoted = true;
			} else {
				throw format_error{ "Only the option 'q' for quoting a path is available." };
			}
		}
		return context.begin();
	}

	auto format(const std::filesystem::path& path, auto& context) {
		if (quoted) {
			return format_to(context.out(), "\"{}\"", nfwk::path_to_string(path));
		} else {
			return format_to(context.out(), "{}", nfwk::path_to_string(path));
		}
	}

private:

	bool quoted{ false };

};

}

#define NFWK_STDSPEC_FORMATTER(ENUM) template<> class std::formatter<ENUM> : public nfwk::stdspec::formatter::enum_formatter<ENUM> {}

// todo: this belongs in utility_functions.hpp, but this header currently depends on it.
// need to split up utility functions into more headers.
NFWK_STDSPEC_FORMATTER(nfwk::file_type);
NFWK_STDSPEC_FORMATTER(nfwk::file_type_category);
