export module nfwk.core:string_functions;

import std.core;
import :utility_functions;

export namespace nfwk {

// todo: split by string should also be possible
std::vector<std::string> split_string(std::string string, char symbol) {
	if (string.empty()) {
		return {};
	}
	std::vector<std::string> result;
	std::size_t start{ 0 };
	std::size_t next{ string.find(symbol) };
	while (next != std::string::npos) {
		result.push_back(string.substr(start, next - start));
		start = next + 1;
		next = string.find(symbol, start);
	}
	result.push_back(string.substr(start, next - start));
	return result;
}

// todo: allow some options like 'all occurrences' or 'last occurrence'
std::string erase_substring(const std::string& string, const std::string& substring) {
	auto result = string;
	if (const auto index = result.find(substring); index != std::string::npos) {
		result.erase(result.find(substring), substring.size());
	}
	return result;
}

void replace_substring(std::string& string, std::string_view substring, std::string_view replace_with) {
	auto index = string.find(substring);
	while (index != std::string::npos) {
		string.replace(index, substring.size(), replace_with);
		index = string.find(substring, index + replace_with.size());
	}
}

std::string string_to_lowercase(std::string string) {
	std::transform(string.begin(), string.end(), string.begin(), [](const auto& character) {
		return std::tolower(character);
	});
	return string;
}

template<typename T, typename U>
std::vector<T> get_map_keys(const std::unordered_map<T, U>& map) {
	std::vector<T> keys;
	for (const auto& [key, element] : map) {
		keys.push_back(key);
	}
	return keys;
}

template<typename Element>
std::vector<std::string> vector_to_strings(const std::vector<Element>& elements) {
	std::vector<std::string> values;
	for (const auto& element : elements) {
		if constexpr (is_pointer<Element>::value) {
			values.push_back(element->to_string());
		} else {
			values.push_back(element.to_string());
		}
	}
	return values;
}

}
