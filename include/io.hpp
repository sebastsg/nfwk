#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <filesystem>
#include <functional>
#include <optional>
#include <unordered_map> // remove when get_map_keys is moved
#include <future> // remove when is_future_ready is moved

#define STRING(X) [&]{ std::ostringstream S; S << X; return S.str(); }()
#define CSTRING(X) STRING(X).c_str()

namespace nfwk {

enum class entry_inclusion { everything, only_files, only_directories };

std::filesystem::path _workaround_fix_windows_path(std::filesystem::path path);

std::vector<std::filesystem::path> entries_in_directory(std::filesystem::path path, entry_inclusion inclusion, bool recursive, const std::function<bool(const std::filesystem::path&)>& predicate = {});

// todo: move these string functions
// todo: split by string should also be possible
std::vector<std::string> split_string(std::string string, char symbol);
// todo: allow some options like 'all occurrences' or 'last occurrence'
std::string erase_substring(const std::string& string, const std::string& substring);
void replace_substring(std::string& string, std::string_view substring, std::string_view replace_with);
std::string string_to_lowercase(std::string string);

// todo: should be moved to a new file.
template<typename T, typename U>
std::vector<T> get_map_keys(const std::unordered_map<T, U>& map) {
	std::vector<T> keys;
	for (const auto& [key, element] : map) {
		keys.push_back(key);
	}
	return keys;
}

class io_stream {
public:

	enum class construct_by { copy, move, shallow_copy };

	io_stream() = default;
	io_stream(std::size_t size);
	io_stream(char* data, std::size_t size, construct_by construction);
	io_stream(const io_stream&) = delete;
	io_stream(io_stream&&) noexcept;
	~io_stream();

	io_stream& operator=(const io_stream&) = delete;
	io_stream& operator=(io_stream&&) noexcept;

	void allocate(std::size_t size);
	void resize(std::size_t new_size);
	void resize_if_needed(std::size_t size_to_write);
	void free();

	// move everything from the read position to the beginning
	// useful when we have extracted some data, and want to 
	// read the rest with a read index of 0
	void shift_read_to_begin();

	void set_read_index(size_t index);
	void set_write_index(size_t index);

	void move_read_index(long long size);
	void move_write_index(long long size);

	bool empty() const;
	std::size_t size() const;
	std::size_t size_left_to_write() const;
	std::size_t size_left_to_read() const;
	std::size_t read_index() const;
	std::size_t write_index() const;

	char* at(std::size_t index) const;
	char* at_read() const;
	char* at_write() const;

	// todo: merge this with peek() instead?
	template<typename T>
	T read(std::size_t index) const {
		if (begin + index > end) {
			return {};
		}
		T value;
		memcpy(&value, begin + index, sizeof(T));
		return value;
	}

	template<typename T>
	T peek() const {
		return read<T>(read_index());
	}

	template<typename T>
	T peek(std::size_t offset) const {
		return read<T>(read_index() + offset);
	}

	template<typename T>
	T read() {
		if (read_position + sizeof(T) > end) {
			return {};
		}
		T value;
		memcpy(&value, read_position, sizeof(T));
		read_position += sizeof(T);
		return value;
	}

	template<>
	std::string read() {
		const std::size_t length{ read<std::uint32_t>() };
		if (length == 0) {
			return "";
		}
		if (read_position + length > end) {
			return {};
		}
		std::string result;
		result.insert(result.begin(), length, '\0');
		std::memcpy(&result[0], read_position, length);
		read_position += length;
		return result;
	}

	template<>
	bool read() {
		return read<std::uint8_t>() != 0;
	}

	void read(char* destination, std::size_t size) {
		if (read_position + size > end) {
			return;
		}
		std::memcpy(destination, read_position, size);
		read_position += size;
	}

	template<typename ReadType, typename ReturnType = ReadType>
	std::optional<ReturnType> read_optional() {
		if (read<bool>()) {
			return static_cast<ReturnType>(read<ReadType>());
		} else {
			return std::nullopt;
		}
	}

	template<typename T>
	void write(T value) {
		resize_if_needed(sizeof(T));
		memcpy(write_position, &value, sizeof(T));
		write_position += sizeof(T);
	}

	template<>
	void write(std::string value) {
		const auto size = value.size();
		write(static_cast<std::uint32_t>(size));
		if (size == 0) {
			return;
		}
		resize_if_needed(size);
		std::memcpy(write_position, value.c_str(), size);
		write_position += size;
	}

	template<>
	void write(bool value) {
		write<std::uint8_t>(value ? 1 : 0);
	}

	void write(const char* source, std::size_t size) {
		resize_if_needed(size);
		std::memcpy(write_position, source, size);
		write_position += size;
	}

	template<typename WriteType, typename SourceType>
	void write_optional(std::optional<SourceType> value) {
		write<bool>(value.has_value());
		if (value.has_value()) {
			write(static_cast<WriteType>(value.value()));
		}
	}

	template<typename WriteType, typename SourceType = WriteType>
	void write_array(const std::vector<SourceType>& values) {
		write(static_cast<std::int32_t>(values.size()));
		for (auto& value : values) {
			write(static_cast<WriteType>(value));
		}
	}

	template<typename WriteType, typename SourceType = WriteType>
	std::vector<WriteType> read_array() {
		std::vector<WriteType> values;
		const auto count = read<std::int32_t>();
		for (std::int32_t i{ 0 }; i < count; i++) {
			values.push_back(static_cast<WriteType>(read<SourceType>()));
		}
		return std::move(values);
	}

	std::size_t read_line(char* destination, std::size_t max_size, bool remove_newline);
	std::string read_line(bool remove_newline);

	int find_first(const std::string& key, std::size_t start) const;
	int find_last(const std::string& key, std::size_t start) const;

	char* data() const;
	bool is_owner() const;

private:

	char* begin{ nullptr };
	char* end{ nullptr };
	char* read_position{ nullptr };
	char* write_position{ nullptr };
	bool owner{ true };

};

class io_streamable {
public:

	virtual void write(io_stream& stream) const = 0;
	virtual void read(io_stream& stream) = 0;

};

void write_file(const std::filesystem::path& path, const std::string& source);
void write_file(const std::filesystem::path& path, const char* source, std::size_t size);
void write_file(const std::filesystem::path& path, io_stream& source);
void append_file(const std::filesystem::path& path, const std::string& source);
std::string read_file(const std::filesystem::path& path);
void read_file(const std::filesystem::path& path, io_stream& destination);

}
