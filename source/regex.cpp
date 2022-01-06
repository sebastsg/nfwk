#include "regex.hpp"
#include "log.hpp"

#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE2_STATIC
#include <pcre2/pcre2.h>

namespace nfwk {

compiled_regex::compiled_regex(std::string_view pattern) {
	int error_code{ 0 };
	PCRE2_SIZE error_offset{ 0 };
	regex_ = pcre2_compile(reinterpret_cast<PCRE2_SPTR8>(pattern.data()), PCRE2_ZERO_TERMINATED, 0, &error_code, &error_offset, nullptr);
	if (regex_) {
		match_data_ = pcre2_match_data_create_from_pattern(static_cast<pcre2_code_8*>(regex_), nullptr);
	} else {
		PCRE2_UCHAR error_string[128]{};
		pcre2_get_error_message(error_code, error_string, sizeof(error_string));
		error("core", "Error compiling regex: %cyan{}%red at %cyan{}", reinterpret_cast<char*>(error_string), error_offset);
	}
}

compiled_regex::~compiled_regex() {
	pcre2_match_data_free(static_cast<pcre2_match_data_8*>(match_data_));
	pcre2_code_free(static_cast<pcre2_code_8*>(regex_));
}

std::optional<std::string_view> compiled_regex::find_in(std::string_view string, std::size_t offset) const {
	if (offset >= string.size()) {
		return std::nullopt;
	}
	const auto* data = string.data() + offset;
	if (const auto size = raw_search(&data, string.size() - offset); size > 0) {
		return string.substr(data - string.data(), size);
	}
	return std::nullopt;
}

std::vector<std::string_view> compiled_regex::find_all_in(std::string_view string) const {
	std::vector<std::string_view> results;
	std::size_t offset{ 0 };
	while (string.size() > offset) {
		if (const auto result = find_in(string, offset)) {
			results.push_back(result.value());
			if (const auto movement = result->data() - string.data() + result->size(); movement > 0) {
				offset += movement;
			} else {
				break;
			}
		} else {
			break;
		}
	}
	return results;
}

std::size_t compiled_regex::raw_search(const char** string, std::size_t size) const {
	if (!regex_ || !match_data_ || !string) {
		*string = nullptr;
		return 0;
	}
	std::size_t length{ 0 };
	auto regex = static_cast<pcre2_code_8*>(regex_);
	auto match_data = static_cast<pcre2_match_data_8*>(match_data_);
	if (const auto rc = pcre2_match(regex, reinterpret_cast<PCRE2_SPTR>(*string), size, 0, 0, match_data, nullptr); rc >= 0) {
		auto ovector = pcre2_get_ovector_pointer(match_data);
		*string += ovector[0];
		length = ovector[1] - ovector[0];
	}
	return length;
}

}
