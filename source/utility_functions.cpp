#include "utility_functions.hpp"
#include "log.hpp"
#include "assert.hpp"

#include <bitset>

#include "imgui_loop_component.hpp"

namespace nfwk {

vector2f scaled_with_aspect(vector2f original_size, vector2f max_new_size) {
	const auto aspect_ratio = original_size.x / original_size.y;
	const vector2f max_width{ max_new_size.x, max_new_size.x / aspect_ratio };
	const vector2f max_height{ max_new_size.y * aspect_ratio, max_new_size.y };
	return max_width.y > max_new_size.y ? max_height : max_width;
}

// todo: split by string should also be possible
std::vector<std::string> split_string(const std::string& string, char symbol) {
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

std::vector<std::string_view> split_string_view(std::string_view string, char symbol) {
	if (string.empty()) {
		return {};
	}
	std::vector<std::string_view> result;
	std::size_t start{ 0 };
	std::size_t next{ string.find(symbol) };
	while (next != std::string_view::npos) {
		result.push_back(string.substr(start, next - start));
		start = next + 1;
		next = string.find(symbol, start);
	}
	result.push_back(string.substr(start, next - start));
	return result;
}

std::string merge_strings(const std::vector<std::string>& strings, std::string_view glue) {
	std::string result;
	result.reserve(strings.size() * 20);
	for (std::size_t i{ 0 }; i < strings.size(); i++) {
		result += strings[i];
		if (i + 1 < strings.size()) {
			result += glue;
		}
	}
	return result;
}

std::string merge_strings(const std::vector<std::string_view>& strings, std::string_view glue) {
	std::string result;
	result.reserve(strings.size() * 20);
	for (std::size_t i{ 0 }; i < strings.size(); i++) {
		result += strings[i];
		if (i + 1 < strings.size()) {
			result += glue;
		}
	}
	return result;
}

std::string string_to_lowercase(std::string string) {
	std::transform(string.begin(), string.end(), string.begin(), [](const auto& character) {
		return std::tolower(character);
	});
	return string;
}

// todo: allow some options like 'all occurrences' or 'last occurrence'

void erase_substring(std::string& string, std::string_view substring, string_comparison comparison) {
	if (substring.empty()) {
		return;
	}
	switch (comparison) {
	case string_comparison::case_sensitive:
		if (const auto index = string.find(substring); index != std::string::npos) {
			string.erase(string.find(substring), substring.size());
			erase_substring(string, substring, comparison);
		}
		break;

	case string_comparison::case_insensitive:
		if (const auto index = string_to_lowercase(string).find(substring); index != std::string::npos) {
			string.erase(index, substring.size());
			erase_substring(string, substring, comparison);
		}
		break;
	}
}

std::size_t replace_substring(std::string& string, std::string_view substring, std::string_view replace_with, std::size_t offset, std::size_t max_replacements) {
	if (substring.empty()) {
		return offset;
	}
	auto new_offset = offset;
	auto index = string.find(substring, offset);
	while (index != std::string::npos && max_replacements != 0) {
		new_offset = index;
		string.replace(index, substring.size(), replace_with);
		index = string.find(substring, index + replace_with.size());
		max_replacements--;
	}
	return new_offset;
}

std::string_view trim_string_view_left(std::string_view string, std::string_view characters) {
	if (const auto offset = string.find_first_not_of(characters.data()); offset != std::string_view::npos) {
		return { string.data() + offset, string.size() - offset };
	} else {
		return {};
	}
}

std::string_view trim_string_view_right(std::string_view string, std::string_view characters) {
	if (const auto offset = string.find_last_not_of(characters.data()); offset != std::string_view::npos) {
		return { string.data(), offset + 1 };
	} else {
		return {};
	}
}

std::string_view trim_string_view(std::string_view string, std::string_view characters) {
	return trim_string_view_left(trim_string_view_right(string, characters), characters);
}

void trim_string_left(std::string& string, std::string_view characters) {
	string.erase(0, string.find_first_not_of(characters.data()));
}

void trim_string_right(std::string& string, std::string_view characters) {
	string.erase(string.find_last_not_of(characters.data()) + 1);
}

void trim_string(std::string& string, std::string_view characters) {
	trim_string_right(string, characters);
	trim_string_left(string, characters);
}

bool string_contains(std::string_view string, std::string_view needle, string_comparison comparison) {
	switch (comparison) {
	case string_comparison::case_sensitive:
		return string.find(needle) != std::string_view::npos;
	case string_comparison::case_insensitive:
		return string_to_lowercase(std::string{ string }).find(string_to_lowercase(std::string{ needle })) != std::string_view::npos;
	}
}

bool string_contains_any(std::string_view string, const std::vector<std::string_view>& needles, string_comparison comparison) {
	switch (comparison) {
	case string_comparison::case_sensitive:
		for (const auto& needle : needles) {
			if (string.find(needle) != std::string_view::npos) {
				return true;
			}
		}
		return false;

	case string_comparison::case_insensitive:
		for (const auto& needle : needles) {
			if (string_to_lowercase(std::string{ string }).find(string_to_lowercase(std::string{ needle })) != std::string_view::npos) {
				return true;
			}
		}
		return false;
	}
}

bool string_equal_to_any(std::string_view string, const std::vector<std::string_view>& needles, string_comparison comparison) {
	switch (comparison) {
	case string_comparison::case_sensitive:
		for (const auto& needle : needles) {
			if (string == needle) {
				return true;
			}
		}
		return false;

	case string_comparison::case_insensitive:
		for (const auto& needle : needles) {
			if (string_to_lowercase(std::string{ string }) == string_to_lowercase(std::string{ needle })) {
				return true;
			}
		}
		return false;
	}
}

void erase_multi_occurrence(std::string& string, std::initializer_list<char> of_characters, char replacement) {
	for (int i{ 0 }; i < static_cast<int>(string.size()) - 1; i++) {
		const bool found_first = std::find(of_characters.begin(), of_characters.end(), string[i]) != of_characters.end();
		const bool found_second = std::find(of_characters.begin(), of_characters.end(), string[i + 1]) != of_characters.end();
		if (found_first && found_second) {
			string.erase(string.begin() + i);
			string[i] = replacement;
			i--;
		}
	}
}

std::string get_filename_from_url(std::string_view url) {
	std::string filename;
	if (const auto last_slash_index = url.rfind('/'); last_slash_index != std::string_view::npos) {
		filename = url.substr(last_slash_index + 1);
	} else if (const auto last_dot_index = url.rfind('.'); last_dot_index != std::string_view::npos) {
		filename = std::format("file-{}{}", std::time(nullptr), url.substr(last_dot_index));
	} else {
		filename = std::format("file-{}", std::time(nullptr));
	}
	replace_with(filename, { '!', '@', '%', '^', '*', '~', '|', '"', '&', '=', '?', '/', '\\', '#' }, '_');
	return filename;
}

static bool has_jpg_header(std::string_view data_) {
	const auto* data = reinterpret_cast<const unsigned char*>(data_.data());
	return data_.size() > 2 && data[0] == 0xff && data[1] == 0xd8 && data[2] == 0xff;
}

static bool has_png_header(std::string_view data_) {
	const auto* data = reinterpret_cast<const unsigned char*>(data_.data());
	return data_.size() > 7 && data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4e && data[3] == 0x47 && data[4] == 0x0d && data[5] == 0x0a && data[6] == 0x1a && data[7] == 0x0a;
}

static bool has_gif_header(std::string_view data) {
	return data.size() > 5 && data[0] == 'G' && data[1] == 'I' && data[2] == 'F' && data[3] == '8' && data[4] == '9' && data[5] == 'a';
}

static bool has_pdf_header(std::string_view data) {
	return data.size() > 4 && data[0] == '%' && data[1] == 'P' && data[2] == 'D' && data[3] == 'F' && data[4] == '-';
}

static bool has_flac_header(std::string_view data) {
	return data.size() > 3 && data[0] == 'f' && data[1] == 'L' && data[2] == 'a' && data[3] == 'C';
}

static bool has_wav_header(std::string_view data) {
	return data.size() > 16 && data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F'
		&& data[8] == 'W' && data[9] == 'A' && data[10] == 'V' && data[11] == 'E';
}

static bool has_ogg_header(std::string_view data) {
	return data.size() > 14;
}

static bool has_mp3_header(std::string_view data) {
	return data.size() > 3 && (*reinterpret_cast<const std::uint32_t*>(&data[0]) & 0x7ff) == 0x7ff;
}

static bool has_aac_header(std::string_view data) {
	return data.size() > 10 && data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x20
		&& data[4] == 'f' && data[5] == 't' && data[6] == 'y' && data[7] == 'p' && data[8] == 'M' && data[9] == '4' && data[10] == 'A';
}

static bool has_webp_header(std::string_view data) {
	return data.size() > 16 && data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F'
		&& data[8] == 'W' && data[9] == 'E' && data[10] == 'B' && data[11] == 'P';
}

static bool has_bmp_header(std::string_view data) {
	return data.size() > 14 && data[0] == 'B' && data[1] == 'M';
}

static bool has_webm_header(std::string_view data) {
	constexpr std::uint32_t ebml_magic{ 0xa3df451a };
	return data.size() > 48 && *reinterpret_cast<const std::uint32_t*>(&data[0]) == ebml_magic
		&& data[31] == 'w' && data[32] == 'e' && data[33] == 'b' && data[34] == 'm';
}

static bool has_mp4_header(std::string_view data) {
	if (data.size() > 10 && data[4] == 'f' && data[5] == 't' && data[6] == 'y' && data[7] == 'p') {
		constexpr std::string_view types[]{
			"avc1", "iso2", "isom", "mmp4", "mp41", "mp42", "mp71", "msnv", "ndas", "ndsc",
			"ndsh", "ndsm", "ndsp", "ndss", "ndxc", "ndxh", "ndxm", "ndxp", "ndxs"
		};
		const std::string_view data_type{ data.data() + 8, 4 };
		return std::ranges::any_of(types, [&data_type](const std::string_view& type) {
			return data_type == type;
		});
	} else {
		return false;
	}
}

static bool has_mkv_header(std::string_view data) {
	constexpr std::uint32_t ebml_magic{ 0xa3df451a };
	if (data.size() < 48 || *reinterpret_cast<const std::uint32_t*>(&data[0]) != ebml_magic) {
		return false;
	}
	constexpr std::string_view matroska{ "matroska" };
	if (matroska == std::string_view{ data.data() + 31, matroska.size() }) {
		return true;
	}
	if (matroska == std::string_view{ data.data() + 8, matroska.size() }) {
		return true;
	}
	return false;
}

static bool has_avi_header(std::string_view data) {
	return data.size() > 16 && data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F'
		&& data[8] == 'A' && data[9] == 'V' && data[10] == 'I' && data[11] == ' ';
}

constexpr auto total_file_types = static_cast<int>(file_type::pdf) + 1;

std::string_view get_file_extension_from_type(file_type type, bool with_dot) {
	switch (type) {
	case file_type::jpg: return with_dot ? ".jpg" : "jpg";
	case file_type::png: return with_dot ? ".png" : "png";
	case file_type::gif: return with_dot ? ".gif" : "gif";
	case file_type::webp: return with_dot ? ".webp" : "webp";
	case file_type::bmp: return with_dot ? ".bmp" : "bmp";
	case file_type::flac: return with_dot ? ".flac" : "flac";
	case file_type::mp3: return with_dot ? ".mp3" : "mp3";
	case file_type::aac: return with_dot ? ".m4a" : "m4a";
	case file_type::wav: return with_dot ? ".wav" : "wav";
	case file_type::ogg: return with_dot ? ".ogg" : "ogg";
	case file_type::webm: return with_dot ? ".webm" : "webm";
	case file_type::mp4: return with_dot ? ".mp4" : "mp4";
	case file_type::mkv: return with_dot ? ".mkv" : "mkv";
	case file_type::avi: return with_dot ? ".avi" : "avi";
	case file_type::pdf: return with_dot ? ".pdf" : "pdf";
	}
}

std::optional<std::string_view> detect_file_extension_from_data(std::string_view data, bool with_dot) {
	if (const auto type = detect_file_type_from_data(data)) {
		return get_file_extension_from_type(type.value(), with_dot);
	} else {
		return std::nullopt;
	}
}

std::optional<file_type> detect_file_type_from_extension(std::string_view extension) {
	if (extension.empty()) {
		return std::nullopt;
	}
	if (const auto dot = extension.rfind('.'); dot != std::string_view::npos) {
		extension = { extension.data() + dot + 1, extension.size() - dot - 1 };
	}
	constexpr auto insensitive_equal = [](char a, char b) {
		return a == std::tolower(b);
	};
	for (int index{ 0 }; index < total_file_types; index++) {
		const auto type = static_cast<file_type>(index);
		const auto current = get_file_extension_from_type(type, false);
		if (std::equal(current.begin(), current.end(), extension.begin(), extension.end(), insensitive_equal)) {
			return type;
		}
		// todo: handle alternate spellings differently
		if (type == file_type::jpg) {
			constexpr std::string_view jpeg{ "jpeg" };
			if (std::equal(jpeg.begin(), jpeg.end(), extension.begin(), extension.end(), insensitive_equal)) {
				return type;
			}
		}
	}
	return std::nullopt;
}

bool detect_if_file_type(std::string_view data, file_type type) {
	switch (type) {
	case file_type::jpg: return has_jpg_header(data);
	case file_type::png: return has_png_header(data);
	case file_type::gif: return has_gif_header(data);
	case file_type::webp: return has_webp_header(data);
	case file_type::bmp: return has_bmp_header(data);
	case file_type::flac: return has_flac_header(data);
	case file_type::mp3: return has_mp3_header(data);
	case file_type::aac: return has_aac_header(data);
	case file_type::wav: return has_wav_header(data);
	case file_type::ogg: return has_ogg_header(data);
	case file_type::webm: return has_webm_header(data);
	case file_type::mp4: return has_mp4_header(data);
	case file_type::mkv: return has_mkv_header(data);
	case file_type::avi: return has_avi_header(data);
	case file_type::pdf: return has_pdf_header(data);
	}
}

std::optional<file_type> detect_file_type_from_data(std::string_view data) {
	for (int type{ 0 }; type < total_file_types; type++) {
		if (detect_if_file_type(data, static_cast<file_type>(type))) {
			return static_cast<file_type>(type);
		}
	}
	return std::nullopt;
}

file_type_category get_file_type_category(file_type type) {
	switch (type) {
	case file_type::jpg:
	case file_type::png:
	case file_type::gif:
	case file_type::webp:
	case file_type::bmp: return file_type_category::image;
	case file_type::flac:
	case file_type::mp3:
	case file_type::aac:
	case file_type::wav:
	case file_type::ogg: return file_type_category::audio;
	case file_type::webm:
	case file_type::mp4:
	case file_type::mkv:
	case file_type::avi: return file_type_category::video;
	case file_type::pdf: return file_type_category::other;
	}
}

std::string_view get_content_type_from_path(const std::filesystem::path& path) {
	constexpr std::pair<std::string_view, std::string_view> content_types[]{
		{ "html", "text/html" },
		{ "css", "text/css" },
		{ "js", "application/javascript" },
		{ "jpg", "image/jpeg" },
		{ "png", "image/png" },
		{ "jpeg", "image/jpeg" },
		{ "webp", "image/webp" },
		{ "svg", "image/svg+xml" },
		{ "gif", "image/gif" },
		{ "bmp", "image/bmp" },
		{ "mp3", "audio/mpeg" },
		{ "m4a", "audio/aac" },
		{ "webm", "video/webm" },
		{ "mp4", "video/mp4" },
		{ "mkv", "video/x-matroska" },
		{ "zip", "application/zip" },
		{ "pdf", "application/pdf" },
		{ "txt", "text/plain" },
	};
	auto extension = path_to_string(path.extension());
	if (!extension.empty() && extension[0] == '.') {
		extension.erase(0, 1);
	}
	for (const auto& [content_extension, content_type] : content_types) {
		if (content_extension == extension) {
			return content_type;
		}
	}
	return "text/plain";
}

std::string find_file_extension(std::filesystem::path path, bool with_dot) {
	for (int type{ 0 }; type < total_file_types; type++) {
		if (std::filesystem::exists(path.replace_extension(get_file_extension_from_type(static_cast<file_type>(type), with_dot)))) {
			return path_to_string(path.extension());
		}
	}
	return {};
}

void unzip_file(const std::filesystem::path& zip_file, const std::filesystem::path& unzip_destination) {
	if constexpr (platform::current_system == platform::supported_system::linux) {
		system_command_manager::run(std::format("unzip \"{}\" -d \"{}\"", zip_file, unzip_destination));
	} else if constexpr (platform::current_system == platform::supported_system::windows) {
		system_command_manager::run(std::format("powershell -command \"Expand-Archive -Force '{}' '{}'\"", zip_file, unzip_destination));
	}
}

bool copy_file(const std::filesystem::path& source, const std::filesystem::path& destination) {
	if (std::error_code error; !std::filesystem::copy_file(source, destination, error)) {
		warning(core::log, "Failed to copy {} to {}. Error {}: {}", source, destination, error.value(), error.message());
		return false;
	} else {
		return true;
	}
}

bool move_file(const std::filesystem::path& source, const std::filesystem::path& destination) {
	std::error_code error;
	std::filesystem::rename(source, destination, error);
	if (error) {
		warning(core::log, "Failed to move {} to {}. Error {}: {}", source, destination, error.value(), error.message());
		return false;
	} else {
		return true;
	}
}

bool delete_file(const std::filesystem::path& path) {
	if (std::error_code error; !std::filesystem::remove(path, error)) {
		warning(core::log, "Failed to delete file {}. Error {}: {}", path, error.value(), error.message());
		return false;
	} else {
		return true;
	}
}

bool delete_directory(const std::filesystem::path& path) {
	std::error_code error;
	if (const auto count = std::filesystem::remove_all(path, error); count == 0) {
		warning(core::log, "Failed to delete directory {}. Error {}: {}", path, error.value(), error.message());
		return false;
	} else {
		return true;
	}
}

// base64 encode/decode adapted from polfosol (5358284). thanks.
// https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c

std::string encode_base64(const unsigned char* p, std::size_t len) {
	std::string result((len + 2) / 3 * 4, '=');
	auto str = &result[0];
	std::size_t j{ 0 };
	std::size_t pad{ len % 3 };
	const std::size_t last{ len - pad };

	constexpr const char* characters{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" };

	for (std::size_t i{ 0 }; i < last; i += 3) {
		const int n{ static_cast<int>(p[i]) << 16 | static_cast<int>(p[i + 1]) << 8 | p[i + 2] };
		str[j++] = characters[n >> 18];
		str[j++] = characters[n >> 12 & 0x3f];
		str[j++] = characters[n >> 6 & 0x3f];
		str[j++] = characters[n & 0x3f];
	}
	if (pad) {
		const int n{ --pad ? static_cast<int>(p[last]) << 8 | p[last + 1] : p[last] };
		str[j++] = characters[pad ? n >> 10 & 0x3f : n >> 2];
		str[j++] = characters[pad ? n >> 4 & 0x3f : n << 4 & 0x3f];
		str[j] = pad ? characters[n << 2 & 0x3f] : '=';
	}
	return result;
}

std::string decode_base64(const unsigned char* p, std::size_t len) {
	if (len == 0) {
		return {};
	}

	constexpr int indices[256]{
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
		0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
		0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
	};
	
	std::size_t j{ 0 };
	std::size_t pad1{ len % 4 || p[len - 1] == '=' };
	std::size_t pad2{ pad1 && (len % 4 > 2 || p[len - 2] != '=') };
	const std::size_t last{ (len - pad1) / 4 << 2 };
	std::string result(last / 4 * 3 + pad1 + pad2, '\0');
	auto str = reinterpret_cast<unsigned char*>(&result[0]);

	for (std::size_t i{ 0 }; i < last; i += 4) {
		const int n{ indices[p[i]] << 18 | indices[p[i + 1]] << 12 | indices[p[i + 2]] << 6 | indices[p[i + 3]] };
		str[j++] = static_cast<unsigned char>(n >> 16);
		str[j++] = n >> 8 & 0xff;
		str[j++] = n & 0xff;
	}
	
	if (pad1) {
		int n{ indices[p[last]] << 18 | indices[p[last + 1]] << 12 };
		str[j++] = static_cast<unsigned char>(n >> 16);
		if (pad2) {
			n |= indices[p[last + 2]] << 6;
			str[j] = n >> 8 & 0xff;
		}
	}
	return result;
}

std::string encode_base64(std::string_view string) {
	return encode_base64(reinterpret_cast<const unsigned char*>(string.data()), string.size());
}

std::string decode_base64(std::string_view base64_string) {
	return decode_base64(reinterpret_cast<const unsigned char*>(base64_string.data()), base64_string.size());
}

}

std::ostream& operator<<(std::ostream& out, nfwk::file_type type) {
	return out << nfwk::get_file_extension_from_type(type, false);
}

std::ostream& operator<<(std::ostream& out, nfwk::file_type_category type) {
	switch (type) {
	using enum nfwk::file_type_category;
	case image: return out << "Image";
	case audio: return out << "Audio";
	case video: return out << "Video";
	case text: return out << "Text";
	case binary: return out << "Binary";
	case other: return out << "Other";
	}
}
