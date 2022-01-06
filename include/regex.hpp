#pragma once

#include <optional>
#include <vector>
#include <string>

namespace nfwk {

class compiled_regex {
public:

	compiled_regex(std::string_view pattern);
	compiled_regex(const compiled_regex&) = delete;
	compiled_regex(compiled_regex&&) = delete;

	~compiled_regex();

	compiled_regex& operator=(const compiled_regex&) = delete;
	compiled_regex& operator=(compiled_regex&&) = delete;

	std::optional<std::string_view> find_in(std::string_view string, std::size_t offset = 0) const;
	std::vector<std::string_view> find_all_in(std::string_view string) const;

private:

	std::size_t raw_search(const char** string, std::size_t size) const;

	void* regex_{ nullptr };
	void* match_data_{ nullptr };

};

}
