#pragma once

#include <cstring>
#include <sstream>
#include <vector>
#include <filesystem>
#include <functional>
#include <optional>
#include <unordered_map> // remove when get_map_keys is moved

namespace nfwk {

inline std::u8string to_string(int value) {
	return reinterpret_cast<const char8_t*>(std::to_string(value).c_str());
}

inline std::u8string to_string(std::string_view string) {
	return reinterpret_cast<const char8_t*>(string.data());
}

inline std::string to_regular_string(std::u8string_view string) {
	return { string.begin(), string.end() };
}

}

namespace nfwk {

enum class entry_inclusion { everything, only_files, only_directories };
enum class size_length { one_byte, two_bytes, four_bytes, eight_bytes };

std::filesystem::path _workaround_fix_windows_path(std::filesystem::path path);

std::vector<std::filesystem::path> entries_in_directory(std::filesystem::path path, entry_inclusion inclusion, bool recursive, const std::function<bool(const std::filesystem::path&)>& predicate = {});

// todo: move these string functions
// todo: split by string should also be possible
std::vector<std::u8string> split_string(const std::u8string& string, char symbol);
// todo: allow some options like 'all occurrences' or 'last occurrence'
std::u8string erase_substring(const std::u8string& string, const std::u8string& substring);
void replace_substring(std::u8string& string, std::u8string_view substring, std::u8string_view replace_with);
std::u8string string_to_lowercase(std::u8string string);

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
	// useful when we have extracted some data, and want to read the rest with a read index of 0
	void shift_read_to_begin();

	void set_read_index(std::size_t index);
	void set_write_index(std::size_t index);

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
		std::memcpy(&value, begin + index, sizeof(T));
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
		static_assert(!std::is_same_v<T, std::string>, "Use read_string()");
		static_assert(!std::is_same_v<T, std::u8string>, "Use read_string()");
		static_assert(!std::is_same_v<T, std::string_view>, "Use read_string(). What are you doing.");
		static_assert(!std::is_same_v<T, std::u8string_view>, "Use read_string(). What are you doing.");
		static_assert(!std::is_same_v<T, bool>, "Use read_bool()");
		if (read_position + sizeof(T) > end) { 
			return {};
		}
		T value;
		std::memcpy(&value, read_position, sizeof(T));
		read_position += sizeof(T);
		return value;
	}

	void write_string(std::u8string_view value) {
		const auto size = value.size();
		write_size(size);
		if (size == 0) {
			return;
		}
		resize_if_needed(size);
		std::memcpy(write_position, value.data(), size);
		write_position += size;
	}
	
	std::u8string read_string() {
		const auto length = read_size();
		if (length == 0 || read_position + length > end) {
			return {};
		}
		std::u8string result;
		result.insert(result.begin(), length, '\0');
		std::memcpy(&result[0], read_position, length);
		read_position += length;
		return result;
	}

	void write_bool(bool value) {
		write<std::uint8_t>(value ? 1 : 0);
	}

	bool read_bool() {
		return read<std::uint8_t>() != 0;
	}

	void read_raw(char* destination, std::size_t size) {
		if (read_position + size > end) {
			return;
		}
		std::memcpy(destination, read_position, size);
		read_position += size;
	}

	template<typename T>
	void write(T value) {
		static_assert(!std::is_same_v<T, std::string_view>, "Use write_string()");
		static_assert(!std::is_same_v<T, std::string>, "Use write_string()");
		static_assert(!std::is_same_v<T, std::u8string_view>, "Use write_string()");
		static_assert(!std::is_same_v<T, std::u8string>, "Use write_string()");
		static_assert(!std::is_same_v<T, bool>, "Use write_bool()");
		resize_if_needed(sizeof(T));
		std::memcpy(write_position, &value, sizeof(T));
		write_position += sizeof(T);
	}

	template<size_length Size = size_length::four_bytes>
	void write_size(std::size_t value) {
		switch (Size) {
		case size_length::one_byte:
			write(static_cast<std::uint8_t>(value));
			break;
		case size_length::two_bytes:
			write(static_cast<std::uint16_t>(value));
			break;
		case size_length::four_bytes:
			write(static_cast<std::uint32_t>(value));
			break;
		case size_length::eight_bytes:
			write(static_cast<std::uint64_t>(value));
			break;
		}
	}
	
	template<size_length Size = size_length::four_bytes>
	std::size_t read_size() {
		switch (Size) {
		case size_length::one_byte: return static_cast<std::size_t>(read<std::uint8_t>());
		case size_length::two_bytes: return static_cast<std::size_t>(read<std::uint16_t>());
		case size_length::four_bytes: return static_cast<std::size_t>(read<std::uint32_t>());
		case size_length::eight_bytes: return static_cast<std::size_t>(read<std::uint64_t>());
		}
	}

	void write_raw(const char* source, std::size_t size) {
		resize_if_needed(size);
		std::memcpy(write_position, source, size);
		write_position += size;
	}

	template<typename ReadType, typename ReturnType = ReadType>
	std::optional<ReturnType> read_optional() {
		if (read_bool()) {
			return static_cast<ReturnType>(read<ReadType>());
		} else {
			return std::nullopt;
		}
	}

	template<typename WriteType, typename SourceType>
	void write_optional(std::optional<SourceType> value) {
		write_bool(value.has_value());
		if (value.has_value()) {
			write(static_cast<WriteType>(value.value()));
		}
	}

	void write_string_array(const std::vector<std::u8string>& values) {
		write_size(values.size());
		for (auto& value : values) {
			write_string(value);
		}
	}

	std::vector<std::u8string> read_string_array() {
		std::vector<std::u8string> values;
		const auto count = read_size();
		values.reserve(count);
		for (std::size_t i{ 0 }; i < count; i++) {
			values.emplace_back(read_string());
		}
		return values;
	}

	template<typename WriteType, typename SourceType = WriteType>
	void write_array(const std::vector<SourceType>& values) {
		write_size(values.size());
		for (auto& value : values) {
			write(static_cast<WriteType>(*this, value));
		}
	}

	template<typename WriteType, typename SourceType = WriteType>
	std::vector<WriteType> read_array() {
		std::vector<WriteType> values;
		const auto count = read_size();
		for (std::size_t i{ 0 }; i < count; i++) {
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

	io_streamable() = default;
	io_streamable(const io_streamable&) = default;
	io_streamable(io_streamable&&) = default;
	
	virtual ~io_streamable() = default;

	io_streamable& operator=(const io_streamable&) = default;
	io_streamable& operator=(io_streamable&&) = default;

	virtual void write(io_stream& stream) const = 0;
	virtual void read(io_stream& stream) = 0;

};

void write_file(const std::filesystem::path& path, std::string_view source);
#ifdef NFWK_CPP_20
void write_file(const std::filesystem::path& path, std::u8string_view source);
#endif

void write_file(const std::filesystem::path& path, const char* source, std::size_t size);
void write_file(const std::filesystem::path& path, io_stream& source);

void append_file(const std::filesystem::path& path, std::string_view source);
#ifdef NFWK_CPP_20
void append_file(const std::filesystem::path& path, std::u8string_view source);
#endif

std::u8string read_file(const std::filesystem::path& path);
void read_file(const std::filesystem::path& path, io_stream& destination);

}
