#include "io.hpp"

#include <fstream>
#include <filesystem>

namespace no {

template<typename DirectoryIterator>
static std::vector<std::filesystem::path> iterate_entries_in_directory(const std::filesystem::path& path, entry_inclusion inclusion) {
	static thread_local std::vector<std::filesystem::path> entries;
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
		entries.push_back(entry.path());
	}
	return entries;
}

std::vector<std::filesystem::path> entries_in_directory(std::filesystem::path path, entry_inclusion inclusion, bool recursive) {
	if (recursive) {
		return iterate_entries_in_directory<std::filesystem::recursive_directory_iterator>(path, inclusion);
	} else {
		return iterate_entries_in_directory<std::filesystem::directory_iterator>(path, inclusion);
	}
}

std::vector<std::string> split_string(std::string string, char symbol) {
	if (string.empty()) {
		return {};
	}
	std::vector<std::string> result;
	size_t start{ 0 };
	size_t next{ string.find(symbol) };
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

io_stream::io_stream(size_t size) {
	allocate(size);
}

io_stream::io_stream(char* data, size_t size, construct_by construction) {
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

void io_stream::resize(size_t new_size) {
	if (!begin) {
		allocate(new_size);
		return;
	}
	const size_t old_read_index{ read_index() };
	const size_t old_write_index{ write_index() };
	char* old_begin{ begin };
	const size_t old_size{ size() };
	const size_t copy_size{ std::min(old_size, new_size) };
	begin = new char[new_size];
	memcpy(begin, old_begin, copy_size);
	delete[] old_begin;
	end = begin;
	if (begin) {
		end += new_size;
	}
	read_position = begin + old_read_index;
	write_position = begin + old_write_index;
}

void io_stream::resize_if_needed(size_t size_to_write) {
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
	const size_t shift_size{ static_cast<size_t>(write_position - read_position) };
	memcpy(begin, read_position, shift_size); // copy read-to-write to begin-to-size.
	read_position = begin;
	write_position = begin + shift_size;
}

void io_stream::set_read_index(size_t index) {
	if (index >= size()) {
		read_position = end;
	} else {
		read_position = begin + index;
	}
}

void io_stream::set_write_index(size_t index) {
	if (index >= size()) {
		write_position = end;
	} else {
		write_position = begin + index;
	}
}

void io_stream::move_read_index(long long size) {
	const long long index{ static_cast<long long>(read_index()) + size };
	set_read_index(static_cast<size_t>(index));
}

void io_stream::move_write_index(long long size) {
	const long long index{ static_cast<long long>(write_index()) + size };
	set_write_index(static_cast<size_t>(index));
}

bool io_stream::empty() const {
	return begin == end;
}

size_t io_stream::size() const {
	return static_cast<size_t>(end - begin);
}

size_t io_stream::size_left_to_write() const {
	return static_cast<size_t>(end - write_position);
}

size_t io_stream::size_left_to_read() const {
	return static_cast<size_t>(write_position - read_position);
}

size_t io_stream::read_index() const {
	return static_cast<size_t>(read_position - begin);
}

size_t io_stream::write_index() const {
	return static_cast<size_t>(write_position - begin);
}

char* io_stream::at(size_t index) const {
	return begin + index;
}

char* io_stream::at_read() const {
	return read_position;
}

char* io_stream::at_write() const {
	return write_position;
}

size_t io_stream::read_line(char* destination, size_t max_size, bool remove_newline) {
	if (size_left_to_read() < 2) {
		return 0;
	}
	size_t i{ 0 };
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
	const size_t end_of_the_line{ i + 1 };
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

int io_stream::find_first(const std::string& key, size_t start) const {
	auto found = strstr(begin + start, key.c_str());
	return found ? static_cast<int>(found - begin) : -1;
}

int io_stream::find_last(const std::string& key, size_t start) const {
	char* found{ begin + start };
	while (found) {
		char* previous{ found };
		found = strstr(found, key.c_str());
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

namespace file {

void write(const std::filesystem::path& path, const std::string& source) {
	std::filesystem::create_directories(path.parent_path());
	if (std::ofstream file{ path, std::ios::binary }; file.is_open()) {
		file << source;
	}
}

void write(const std::filesystem::path& path, const char* source, size_t size) {
	std::filesystem::create_directories(path.parent_path());
	if (std::ofstream file{ path, std::ios::binary }; file.is_open()) {
		file.write(source, size);
	}
}

void write(const std::filesystem::path& path, io_stream& source) {
	write(path, source.data(), source.write_index());
}

void append(const std::filesystem::path& path, const std::string& source) {
	if (std::ofstream file{ path, std::ios::app }; file.is_open()) {
		file << source;
	}
}

std::string read(const std::filesystem::path& path) {
	if (std::ifstream file{ path, std::ios::binary }; file.is_open()) {
		std::stringstream result;
		result << file.rdbuf();
		return result.str();
	} else {
		return "";
	}
}

void read(const std::filesystem::path& path, io_stream& stream) {
	if (std::ifstream file{ path, std::ios::binary }; file.is_open()) {
		std::stringstream result;
		result << file.rdbuf();
		const auto& string = result.str();
		stream.write(string.c_str(), string.size());
	}
}

}

}
