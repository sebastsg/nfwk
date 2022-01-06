#pragma once

#include "vector2.hpp"

#include <future>
#include <vector>
#include <charconv>
#include <optional>
#include <filesystem>

namespace nfwk {

inline std::string u8string_to_string(std::u8string_view string) {
	return { string.begin(), string.end() };
}

inline std::string path_to_string(const std::filesystem::path& path) {
	return u8string_to_string(path.u8string());
}

inline std::string from_u8(std::u8string_view string) {
	return u8string_to_string(string);
}

template<typename Enum>
std::string enum_string(Enum value) {
	std::stringstream stream;
	stream << value;
	return stream.str();
}

template<typename Enum>
std::string enum_string(const std::optional<Enum>& value) {
	if (value.has_value()) {
		std::stringstream stream;
		stream << value.value();
		return stream.str();
	} else {
		return {};
	}
}

enum class string_comparison { case_sensitive, case_insensitive };

template<typename T>
[[nodiscard]] bool is_future_ready(const std::future<T>& future) {
	return future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

template<typename T>
[[nodiscard]] std::vector<T> merge_vectors(const std::vector<T>& front, const std::vector<T>& back) {
	std::vector<T> result;
	result.reserve(front.size() + back.size());
	result.insert(result.end(), front.begin(), front.end());
	result.insert(result.end(), back.begin(), back.end());
	return result;
}

vector2f scaled_with_aspect(vector2f original_size, vector2f max_new_size);

template<typename T>
struct is_pointer {
	static constexpr bool value{ false };
};

template<typename T>
struct is_pointer<T*> {
	static constexpr bool value{ true };
};

template<typename T>
struct is_pointer<std::shared_ptr<T>> {
	static constexpr bool value{ true };
};

template<typename Element, typename String = std::string>
[[nodiscard]] std::vector<String> to_strings(const std::vector<Element>& elements) {
	std::vector<String> values;
	for (const auto& element : elements) {
		if constexpr (is_pointer<Element>::value) {
			values.push_back(element->to_string());
		} else if constexpr (std::is_arithmetic_v<Element>) {
			values.push_back(std::to_string(element));
		} else {
			values.push_back(element.to_string());
		}
	}
	return values;
}

// todo: split by string should also be possible
[[nodiscard]] std::vector<std::string> split_string(const std::string& string, char symbol);
[[nodiscard]] std::vector<std::string_view> split_string_view(std::string_view string, char symbol);

[[nodiscard]] std::string merge_strings(const std::vector<std::string_view>& strings, std::string_view glue);
[[nodiscard]] std::string merge_strings(const std::vector<std::string>& strings, std::string_view glue);

[[nodiscard]] std::string string_to_lowercase(std::string string);

// todo: allow some options like 'all occurrences' or 'last occurrence'

using resize_function = std::function<std::pair<char*, std::size_t>(std::size_t extra_size_needed)>;

void erase_substring(std::string& string, std::string_view substring, string_comparison comparison = string_comparison::case_sensitive);
std::size_t replace_substring(std::string& string, std::string_view substring, std::string_view replace_with, std::size_t offset = 0, std::size_t max_replacements = std::string::npos);

// todo: should be moved to a new file.
template<typename T, typename U>
[[nodiscard]] std::vector<T> get_map_keys(const std::unordered_map<T, U>& map) {
	std::vector<T> keys;
	for (const auto& [key, element] : map) {
		keys.push_back(key);
	}
	return keys;
}

template<typename T>
[[nodiscard]] std::optional<T> from_string(std::string_view string) {
	T result{};
	auto data = reinterpret_cast<const char*>(string.data());
	const auto status = std::from_chars(data, data + string.size(), result);
	return status.ec == std::errc{} ? result : std::optional<T>{};
}

[[nodiscard]] std::string_view trim_string_view_left(std::string_view string, std::string_view characters);
[[nodiscard]] std::string_view trim_string_view_right(std::string_view string, std::string_view characters);
[[nodiscard]] std::string_view trim_string_view(std::string_view string, std::string_view characters);

void trim_string_left(std::string& string, std::string_view characters);
void trim_string_right(std::string& string, std::string_view characters);
void trim_string(std::string& string, std::string_view characters);

bool string_contains(std::string_view string, std::string_view needle, string_comparison comparison = string_comparison::case_sensitive);
bool string_contains_any(std::string_view string, const std::vector<std::string_view>& needles, string_comparison comparison = string_comparison::case_sensitive);
bool string_equal_to_any(std::string_view string, const std::vector<std::string_view>& needles, string_comparison comparison = string_comparison::case_sensitive);

void erase_multi_occurrence(std::string& string, std::initializer_list<char> of_characters, char replacement);

template<typename Type, typename SubType>
void replace_with(Type& element, std::initializer_list<SubType> of_these, SubType with_this) {
	for (auto& of_element : element) {
		for (const auto& of_this : of_these) {
			if (of_element == of_this) {
				of_element = with_this;
			}
		}
	}
}

std::string get_filename_from_url(std::string_view url);

enum class file_type {
	// images
	jpg,
	png,
	gif,
	webp,
	bmp,

	// audio
	flac,
	mp3,
	aac,
	wav,
	ogg,

	// video
	webm,
	mp4,
	mkv,
	avi,

	// other
	pdf // note: pdf must always be last, as it has the great honor of being used as a counter.
};

enum class file_type_category {
	image,
	audio,
	video,
	text,
	binary,
	other
};

std::string_view get_file_extension_from_type(file_type type, bool with_dot);
std::optional<std::string_view> detect_file_extension_from_data(std::string_view data, bool with_dot);
std::optional<file_type> detect_file_type_from_extension(std::string_view extension);

bool detect_if_file_type(std::string_view data, file_type type);
std::optional<file_type> detect_file_type_from_data(std::string_view data);

file_type_category get_file_type_category(file_type type);

std::string_view get_content_type_from_path(const std::filesystem::path& path);

// todo: remove function:
std::string find_file_extension(std::filesystem::path path, bool with_dot);

void unzip_file(const std::filesystem::path& zip_file, const std::filesystem::path& unzip_destination);

bool copy_file(const std::filesystem::path& source, const std::filesystem::path& destination);
bool move_file(const std::filesystem::path& source, const std::filesystem::path& destination);

bool delete_file(const std::filesystem::path& path);
bool delete_directory(const std::filesystem::path& path);

std::string encode_base64(std::string_view string);
std::string decode_base64(std::string_view base64_string);

}

std::ostream& operator<<(std::ostream& out, nfwk::file_type type);
std::ostream& operator<<(std::ostream& out, nfwk::file_type_category type);
