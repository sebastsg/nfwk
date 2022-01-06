#include "io.hpp"
#include "log.hpp"

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
		auto path_string = path.u8string();
		if (const auto index = path_string.find(':'); index != std::string::npos) {
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

io_stream::io_stream(size_type size) {
	allocate(size);
}

io_stream::io_stream(char* data, size_type size, construct_by construction) {
	switch (construction) {
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

io_stream::io_stream(const char* data, size_type size, const_construct_by construction) {
	switch (construction) {
	case const_construct_by::copy:
		write_raw(data, size);
		break;
	case const_construct_by::shallow_copy:
		begin = const_cast<char*>(data); // note: don't worry -- never modified, because owner is false.
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
	if (owner) {
		delete[] begin;
	}
}

io_stream& io_stream::operator=(io_stream&& that) noexcept {
	std::swap(begin, that.begin);
	std::swap(end, that.end);
	std::swap(read_position, that.read_position);
	std::swap(write_position, that.write_position);
	std::swap(owner, that.owner);
	return *this;
}

void io_stream::allocate(size_type size) {
	if (begin) {
		resize(size);
		return;
	}
	try {
		begin = new char[size];
	} catch (const std::bad_alloc& e) {
		error("core", "Failed to allocate {} bytes. Error: {}", size, e.what());
		begin = nullptr;
	}
	end = begin;
	if (begin) {
		end += size;
	}
	read_position = begin;
	write_position = begin;
}

void io_stream::resize(size_type new_size) {
	if (!begin) {
		allocate(new_size);
		return;
	}
	const auto old_read_index = read_index();
	const auto old_write_index = write_index();
	char* old_begin{ begin };
	const auto old_size = size();
	const auto copy_size = std::min(old_size, new_size);
	begin = new char[new_size];
	std::memcpy(begin, old_begin, copy_size);
	if (owner) {
		delete[] old_begin;
	} else {
		owner = true;
	}
	end = begin;
	if (begin) {
		end += new_size;
	}
	read_position = begin + old_read_index;
	write_position = begin + old_write_index;
}

void io_stream::resize_if_needed(size_type size_to_write) {
	if (size_to_write > size_left_to_write()) {
		// size() might be 0, so make sure the data fits comfortably.
		const auto new_size = size() * 2 + size_to_write + 64;
		resize(new_size);
	}
}

void io_stream::shift_read_to_begin() {
	const auto shift_size = static_cast<std::size_t>(write_position - read_position);
	std::memcpy(begin, read_position, shift_size); // copy read-to-write to begin-to-size.
	read_position = begin;
	write_position = begin + shift_size;
}

void io_stream::set_read_index(size_type index) {
	if (index >= size()) {
		read_position = end;
	} else {
		read_position = begin + index;
	}
}

void io_stream::set_write_index(size_type index) {
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

std::optional<io_stream::size_type> io_stream::replace(std::string_view search_for, std::string_view replace_with, size_type offset, size_type max_replacements) {
	if (offset >= size_left_to_read()) {
		return std::nullopt;
	}
	size_type replacements{ 0 };
	while (max_replacements-- != 0) {
		std::string_view search_view{ reinterpret_cast<char*>(at_read()) + offset, static_cast<std::string_view::size_type>(size_left_to_read() - offset) };
		auto it = std::search(search_view.begin(), search_view.end(), search_for.begin(), search_for.end());
		if (it == search_view.end()) {
			break;
		}
		auto found = const_cast<char*>(&*it);
		offset = found - reinterpret_cast<char*>(at_read());
		resize_if_needed(replace_with.size());
		found = reinterpret_cast<char*>(at_read()) + offset;
		std::memmove(found + replace_with.size(), found + search_for.size(), size_left_to_read() - offset);
		std::memcpy(found, replace_with.data(), replace_with.size());
		move_write_index(static_cast<long long>(replace_with.size()) - static_cast<long long>(search_for.size()));
		offset += replace_with.size();
		replacements++;
	}
	return replacements > 0 ? offset : std::optional<size_type>{};
}

bool io_stream::empty() const {
	return write_position == read_position;
}

io_stream::size_type io_stream::size() const {
	return static_cast<size_type>(end - begin);
}

io_stream::size_type io_stream::size_left_to_write() const {
	return static_cast<size_type>(end - write_position);
}

io_stream::size_type io_stream::size_left_to_read() const {
	return static_cast<size_type>(write_position - read_position);
}

io_stream::size_type io_stream::read_index() const {
	return static_cast<size_type>(read_position - begin);
}

io_stream::size_type io_stream::write_index() const {
	return static_cast<size_type>(write_position - begin);
}

//char* io_stream::at(std::size_t index) const {
//	return read_position + index;
//}

char* io_stream::at_read() const {
	return read_position;
}

char* io_stream::at_write() const {
	return write_position;
}

void io_stream::write_string(std::string_view value) {
	const auto size = value.size();
	write_size(size);
	if (size == 0) {
		return;
	}
	resize_if_needed(size);
	std::memcpy(write_position, value.data(), size);
	write_position += size;
}

std::string io_stream::read_string() {
	const auto length = read_size();
	if (length == 0 || read_position + length > end) {
		return {};
	}
	std::string result;
	result.insert(result.begin(), length, '\0');
	std::memcpy(&result[0], read_position, length);
	read_position += length;
	return result;
}

void io_stream::write_raw(std::string_view buffer) {
	write_raw(buffer.data(), buffer.size());
}

void io_stream::write_raw(const void* source, size_type size) {
	resize_if_needed(size);
	std::memcpy(write_position, source, size);
	write_position += size;
}

void io_stream::read_raw(void* destination, size_type size) {
	if (end >= read_position + size) {
		std::memcpy(destination, read_position, size);
		read_position += size;
	}
}

void io_stream::write_stream(const io_stream& stream) {
	write_raw(stream.data(), stream.write_index());
}

void io_stream::write_bool(bool value) {
	write_int8(value ? 1 : 0);
}

bool io_stream::read_bool() {
	return read_int8() != 0;
}

void io_stream::write_string_array(const std::vector<std::string>& values) {
	write_size(values.size());
	for (auto& value : values) {
		write_string(value);
	}
}

std::vector<std::string> io_stream::read_string_array() {
	std::vector<std::string> values;
	const auto count = read_size();
	values.reserve(count);
	for (std::size_t i{ 0 }; i < count; i++) {
		values.emplace_back(read_string());
	}
	return values;
}

io_stream::size_type io_stream::read_line(char* destination, size_type max_size, bool remove_newline) {
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

io_stream::size_type io_stream::find_first(std::string_view key, size_type start) const {
	return get_string_view().find(key, start);
}

io_stream::size_type io_stream::find_last(std::string_view key, size_type start) const {
	return get_string_view().rfind(key, start);
}

char* io_stream::data() const {
	return begin;
}

bool io_stream::is_owner() const {
	return owner;
}

void io_stream::reset() {
	read_position = begin;
	write_position = begin;
}

void write_file(const std::filesystem::path& path, std::string_view source) {
	std::error_code error{};
	std::filesystem::create_directories(path.parent_path(), error);
	if (std::ofstream file{ path, std::ios::binary }; file.is_open()) {
		file << source;
	}
}

void write_file(const std::filesystem::path& path, const char* source, std::uintmax_t size) {
	std::error_code error{};
	std::filesystem::create_directories(path.parent_path(), error);
	if (std::ofstream file{ path, std::ios::binary }; file.is_open()) {
		file.write(source, size);
	}
}

void write_file(const std::filesystem::path& path, io_stream& source) {
	write_file(path, source.at_read(), source.size_left_to_read());
}

void append_file(const std::filesystem::path& path, std::string_view source) {
	if (std::ofstream file{ path, std::ios::app }; file.is_open()) {
		file << source;
	}
}

std::string read_file(const std::filesystem::path& path) {
	if (const std::ifstream file{ path, std::ios::binary }; file.is_open()) {
		std::stringstream result;
		result << file.rdbuf();
		return result.str();
	} else {
		return {};
	}
}

void read_file(const std::filesystem::path& path, io_stream& destination) {
	if (const std::ifstream file{ path, std::ios::binary }; file.is_open()) {
		std::stringstream result;
		result << file.rdbuf();
		const auto& string = result.str();
		destination.write_raw(string.c_str(), string.size());
	}
}

io_stream file_section_stream(const std::filesystem::path& path, std::uintmax_t offset, std::uintmax_t max_size) {
	if (std::ifstream file{ path, std::ios::binary }; file.is_open()) {
		std::error_code size_error;
		const auto file_size = std::min(std::filesystem::file_size(path, size_error), max_size);
		if (size_error) {
			return {};
		}
		file.seekg(offset);
		io_stream stream{ file_size };
		file.read(stream.data(), file_size);
		stream.set_read_index(static_cast<io_stream::size_type>(file.gcount()));
		return stream;
	} else {
		return {};
	}
}

}
