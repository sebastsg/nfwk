#include "io.hpp"

#include <fstream>
#include <filesystem>

namespace nfwk {

template<typename DirectoryIterator>
static std::vector<std::filesystem::path> iterate_entries_in_directory(const std::filesystem::path& path, entry_inclusion inclusion, const std::function<bool(const std::filesystem::path&)>& predicate) {
	thread_local std::vector<std::filesystem::path> entries;
	entries.clear();
	std::error_code error_code{};
	for (const auto& entry : DirectoryIterator{ path, std::filesystem::directory_options::skip_permission_denied, error_code }) {
		if (inclusion != entry_inclusion::everything) {
			if (entry.is_directory() && inclusion == entry_inclusion::only_files) {
				continue;
			}
			if (!entry.is_directory() && inclusion == entry_inclusion::only_directories) {
				continue;
			}
		}
		if (!predicate || predicate(entry)) {
			entries.push_back(entry);
		}
	}
	return entries;
}

std::filesystem::path _workaround_fix_windows_path(std::filesystem::path path) {
	// todo: this is a visual c++ bug, so check if this is fixed later.
	// according to "decltype(auto)" in c++ discord, this is the problem: https://github.com/microsoft/STL/blob/main/stl/inc/filesystem#L2624
	// "As for the fact that it lists the stuff from your workdir even though you specified C:, well, thank FindFirstFileExW for not being so nice to use to list root-dirs"
	if (path.u8string().back() == ':') {
		path /= "/";
	} else {
		std::string path_string = path.u8string();
		if (const std::size_t index = path_string.find(':'); index != std::string::npos) {
			if (index + 1 < path_string.size()) {
				if (path_string[index + 1] != '/' && path_string[index + 1] != '\\') {
					path_string.insert(path_string.begin() + index + 1, '/');
					path = path_string;
				}
			}
		}
	}
	return path;
}

std::vector<std::filesystem::path> entries_in_directory(std::filesystem::path path, entry_inclusion inclusion, bool recursive, const std::function<bool(const std::filesystem::path&)>& predicate) {
	if (path.empty()) {
		return {}; // todo: return root directories?
	}
	path = _workaround_fix_windows_path(path);
	if (recursive) {
		return iterate_entries_in_directory<std::filesystem::recursive_directory_iterator>(path, inclusion, predicate);
	} else {
		return iterate_entries_in_directory<std::filesystem::directory_iterator>(path, inclusion, predicate);
	}
}

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

io_stream::io_stream(std::size_t size) {
	allocate(size);
}

io_stream::io_stream(char* data, std::size_t size, construct_by construction) {
	switch (construction) {
	case construct_by::copy:
		write(data, size);
		break;
	case construct_by::move:
		begin = data;
		end = begin + size;
		read_position = begin;
		write_position = end;
		break;
	case construct_by::shallow_copy:
		begin = data;
		end = begin + size;
		read_position = begin;
		write_position = end;
		owner = false;
		break;
	}
}

io_stream::io_stream(io_stream&& that) noexcept {
	std::swap(begin, that.begin);
	std::swap(end, that.end);
	std::swap(read_position, that.read_position);
	std::swap(write_position, that.write_position);
	std::swap(owner, that.owner);
}

io_stream::~io_stream() {
	free();
}

io_stream& io_stream::operator=(io_stream&& that) noexcept {
	std::swap(begin, that.begin);
	std::swap(end, that.end);
	std::swap(read_position, that.read_position);
	std::swap(write_position, that.write_position);
	std::swap(owner, that.owner);
	return *this;
}

void io_stream::allocate(size_t size) {
	if (begin) {
		resize(size);
		return;
	}
	begin = new char[size];
	end = begin;
	if (begin) {
		end += size;
	}
	read_position = begin;
	write_position = begin;
}

void io_stream::resize(std::size_t new_size) {
	if (!begin) {
		allocate(new_size);
		return;
	}
	const std::size_t old_read_index{ read_index() };
	const std::size_t old_write_index{ write_index() };
	char* old_begin{ begin };
	const std::size_t old_size{ size() };
	const std::size_t copy_size{ std::min(old_size, new_size) };
	begin = new char[new_size];
	std::memcpy(begin, old_begin, copy_size);
	delete[] old_begin;
	end = begin;
	if (begin) {
		end += new_size;
	}
	read_position = begin + old_read_index;
	write_position = begin + old_write_index;
}

void io_stream::resize_if_needed(std::size_t size_to_write) {
	if (size_to_write > size_left_to_write()) {
		// size() might be 0, so make sure the data fits comfortably.
		const size_t new_size{ size() * 2 + size_to_write + 64 };
		resize(new_size);
	}
}

void io_stream::free() {
	if (owner) {
		delete[] begin;
	}
	begin = nullptr;
	end = nullptr;
	read_position = nullptr;
	write_position = nullptr;
}

void io_stream::shift_read_to_begin() {
	const auto shift_size = static_cast<std::size_t>(write_position - read_position);
	std::memcpy(begin, read_position, shift_size); // copy read-to-write to begin-to-size.
	read_position = begin;
	write_position = begin + shift_size;
}

void io_stream::set_read_index(std::size_t index) {
	if (index >= size()) {
		read_position = end;
	} else {
		read_position = begin + index;
	}
}

void io_stream::set_write_index(std::size_t index) {
	if (index >= size()) {
		write_position = end;
	} else {
		write_position = begin + index;
	}
}

void io_stream::move_read_index(long long size) {
	const auto index = static_cast<long long>(read_index()) + size;
	set_read_index(static_cast<std::size_t>(index));
}

void io_stream::move_write_index(long long size) {
	const auto index = static_cast<long long>(write_index()) + size;
	set_write_index(static_cast<std::size_t>(index));
}

bool io_stream::empty() const {
	return begin == end;
}

std::size_t io_stream::size() const {
	return static_cast<std::size_t>(end - begin);
}

std::size_t io_stream::size_left_to_write() const {
	return static_cast<std::size_t>(end - write_position);
}

std::size_t io_stream::size_left_to_read() const {
	return static_cast<std::size_t>(write_position - read_position);
}

std::size_t io_stream::read_index() const {
	return static_cast<std::size_t>(read_position - begin);
}

std::size_t io_stream::write_index() const {
	return static_cast<std::size_t>(write_position - begin);
}

char* io_stream::at(std::size_t index) const {
	return begin + index;
}

char* io_stream::at_read() const {
	return read_position;
}

char* io_stream::at_write() const {
	return write_position;
}

std::size_t io_stream::read_line(char* destination, std::size_t max_size, bool remove_newline) {
	if (size_left_to_read() < 2) {
		return 0;
	}
	std::size_t i{ 0 };
	while (read_position[i] != '\n') {
		destination[i] = read_position[i];
		if (destination[i] == '\0') {
			read_position += i;
			return i;
		}
		++i;
		if (i >= max_size) {
			read_position += i;
			destination[max_size - 1] = '\0';
			return max_size - 1;
		}
	}
	const std::size_t end_of_the_line{ i + 1 };
	if (remove_newline) {
		--i; // remove last increment
		if (i - 1 > 0 && read_position[i - 2] == '\r') {
			--i; // remove \r
		}
	} else {
		destination[i] = '\n';
	}
	memcpy(destination, read_position, i);
	read_position += end_of_the_line;
	destination[i] = '\0';
	return i;
}

std::string io_stream::read_line(bool remove_newline) {
	std::string result;
	char buffer[256];
	while (read_line(buffer, 256, remove_newline) != 0) {
		result += buffer;
	}
	return result;
}

int io_stream::find_first(const std::string& key, std::size_t start) const {
	auto found = std::strstr(begin + start, key.c_str());
	return found ? static_cast<int>(found - begin) : -1;
}

int io_stream::find_last(const std::string& key, std::size_t start) const {
	char* found{ begin + start };
	while (found) {
		char* previous{ found };
		found = std::strstr(found, key.c_str());
		if (!found) {
			return static_cast<int>(previous - begin);
		}
		++found;
	}
	return -1;
}

char* io_stream::data() const {
	return begin;
}

bool io_stream::is_owner() const {
	return owner;
}

void write_file(const std::filesystem::path& path, const std::string& source) {
	std::filesystem::create_directories(path.parent_path());
	if (std::ofstream file{ path, std::ios::binary }; file.is_open()) {
		file << source;
	}
}

void write_file(const std::filesystem::path& path, const char* source, std::size_t size) {
	std::filesystem::create_directories(path.parent_path());
	if (std::ofstream file{ path, std::ios::binary }; file.is_open()) {
		file.write(source, size);
	}
}

void write_file(const std::filesystem::path& path, io_stream& source) {
	write_file(path, source.data(), source.write_index());
}

void append_file(const std::filesystem::path& path, const std::string& source) {
	if (std::ofstream file{ path, std::ios::app }; file.is_open()) {
		file << source;
	}
}

std::string read_file(const std::filesystem::path& path) {
	if (std::ifstream file{ path, std::ios::binary }; file.is_open()) {
		std::stringstream result;
		result << file.rdbuf();
		return result.str();
	} else {
		return "";
	}
}

void read_file(const std::filesystem::path& path, io_stream& stream) {
	if (std::ifstream file{ path, std::ios::binary }; file.is_open()) {
		std::stringstream result;
		result << file.rdbuf();
		const auto& string = result.str();
		stream.write(string.c_str(), string.size());
	}
}

}
