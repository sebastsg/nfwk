#pragma once

#include "utility_functions.hpp"
#include "formatting.hpp"

#include <cstring>
#include <sstream>
#include <vector>
#include <filesystem>
#include <functional>
#include <optional>
#include <format>

namespace nfwk {

enum class entry_inclusion { everything, only_files, only_directories };
enum class size_length { one_byte, two_bytes, four_bytes, eight_bytes };

std::filesystem::path _workaround_fix_windows_path(std::filesystem::path path);

std::vector<std::filesystem::path> entries_in_directory(std::filesystem::path path, entry_inclusion inclusion, bool recursive, const std::function<bool(const std::filesystem::path&)>& predicate = {});

class io_stream {
public:

	using size_type = std::uintmax_t;

	enum class construct_by { move, shallow_copy };
	enum class const_construct_by { copy, shallow_copy };

	io_stream() = default;
	io_stream(size_type size);
	io_stream(char* data, size_type size, construct_by construction);
	io_stream(const char* data, size_type size, const_construct_by construction);
	io_stream(const io_stream&) = delete;
	io_stream(io_stream&&) noexcept;
	
	~io_stream();

	io_stream& operator=(const io_stream&) = delete;
	io_stream& operator=(io_stream&&) noexcept;

	std::string_view get_string_view() const {
		return { at_read(), static_cast<std::string_view::size_type>(size_left_to_read()) };
	}

	std::string_view get_string_view(size_type from, size_type to) const {
		return { at_read() + from, static_cast<std::string_view::size_type>(to - from) };
	}

	void allocate(size_type size);
	void resize(size_type new_size);
	void resize_if_needed(size_type size_to_write);

	// move everything from the read position to the beginning
	// useful when we have extracted some data, and want to read the rest with a read index of 0
	void shift_read_to_begin();

	void set_read_index(size_type index);
	void set_write_index(size_type index);

	void move_read_index(long long size);
	void move_write_index(long long size);

	std::optional<size_type> replace(std::string_view search_for, std::string_view replace_with, size_type offset = 0, size_type max_replacements = std::string::npos);

	[[nodiscard]] bool empty() const;
	[[nodiscard]] size_type size() const;
	[[nodiscard]] size_type size_left_to_write() const;
	[[nodiscard]] size_type size_left_to_read() const;
	[[nodiscard]] size_type read_index() const;
	[[nodiscard]] size_type write_index() const;

	//char* at(size_type index) const;
	char* at_read() const;
	char* at_write() const;

	void write_string(std::string_view value);
	std::string read_string();

	void write_raw(std::string_view buffer);
	void write_raw(const void* source, size_type size);
	void read_raw(void* destination, size_type size);

	void write_stream(const io_stream& stream);

	template<typename Type>
	Type peek(size_type offset = 0) const {
		const auto* const peek_at = begin + read_index() + offset;
		if (peek_at > end) {
			return {};
		}
		Type value;
		std::memcpy(&value, peek_at, sizeof(Type));
		return value;
	}

	template<typename DestinationType, typename SourceType>
	void write_trivial(SourceType value) {
		static_assert(std::is_trivially_copyable_v<SourceType>, "SourceType must be trivially copyable.");
		static_assert(std::is_trivially_copyable_v<DestinationType>, "DestinationType must be trivially copyable.");
		if constexpr (sizeof(DestinationType) == sizeof(SourceType)) {
			write_raw(&value, sizeof(DestinationType));
		} else {
			const auto casted_value = static_cast<DestinationType>(value);
			write_raw(&casted_value, sizeof(DestinationType));
		}
	}

	template<typename SourceType, typename DestinationType = SourceType>
	DestinationType read_trivial() {
		static_assert(std::is_trivially_copyable_v<SourceType>, "SourceType must be trivially copyable.");
		static_assert(sizeof(DestinationType) >= sizeof(SourceType), "SourceType can't be bigger than DestinationType.");
		SourceType source;
		read_raw(&source, sizeof(SourceType));
		if constexpr (std::is_same_v<SourceType, DestinationType>) {
			return source;
		} else {
			return static_cast<DestinationType>(source);
		}
	}

	template<typename DestinationType, typename SourceType = DestinationType>
	void write_array(const SourceType* values, size_type count) {
		static_assert(sizeof(SourceType) == sizeof(DestinationType), "SourceType and DestinationType size must be equal.");
		static_assert(std::is_trivially_copyable_v<SourceType>, "SourceType must be trivially copyable.");
		static_assert(std::is_trivially_copyable_v<DestinationType>, "DestinationType must be trivially copyable.");
		write_size(count);
		if (values) {
			write_raw(values, sizeof(DestinationType) * count);
		}
	}

	template<typename DestinationType, typename SourceType = DestinationType>
	void write_array(const std::vector<SourceType>& values) {
		write_array<DestinationType, SourceType>(values.data(), values.size());
	}

	template<typename SourceType, typename DestinationType = SourceType>
	std::vector<DestinationType> read_array() {
		static_assert(sizeof(SourceType) == sizeof(DestinationType), "SourceType and DestinationType size must be equal.");
		static_assert(std::is_trivially_copyable_v<SourceType>, "SourceType must be trivially copyable.");
		static_assert(std::is_trivially_copyable_v<DestinationType>, "DestinationType must be trivially copyable.");
		const auto count = read_size();
		std::vector<DestinationType> values(count);
		read_raw(values.data(), count * sizeof(SourceType));
		return values;
	}

	template<typename DestinationType, typename SourceType>
	void write_float(SourceType value) {
		static_assert(std::is_floating_point_v<DestinationType>, "DestinationType must be floating point.");
		write_trivial<DestinationType, SourceType>(value);
	}

	template<typename SourceType, typename DestinationType = SourceType>
	DestinationType read_float() {
		static_assert(std::is_floating_point_v<SourceType>, "SourceType must be floating point.");
		return read_trivial<SourceType, DestinationType>();
	}

	template<typename DestinationType, typename SourceType>
	void write_int(SourceType value) {
		static_assert(std::is_integral_v<DestinationType>, "DestinationType must be integral.");
		write_trivial<DestinationType, SourceType>(value);
	}

	template<typename SourceType, typename DestinationType = SourceType>
	DestinationType read_int() {
		static_assert(std::is_integral_v<SourceType>, "SourceType must be integral.");
		return read_trivial<SourceType, DestinationType>();
	}

	template<typename DestinationType, typename SourceType>
	void write_optional(std::optional<SourceType> value) {
		write_bool(value.has_value());
		if (value.has_value()) {
			write_trivial<DestinationType, SourceType>(value.value());
		}
	}

	template<typename SourceType, typename DestinationType = SourceType>
	std::optional<DestinationType> read_optional() {
		if (read_bool()) {
			return read_trivial<SourceType, DestinationType>();
		} else {
			return std::nullopt;
		}
	}

	template<typename SourceType>
	void write_float64(SourceType value) {
		static_assert(sizeof(double) == 8, "sizeof(double) must be 64 bits.");
		write_float<double>(value);
	}

	template<typename SourceType>
	void write_float32(SourceType value) {
		static_assert(sizeof(float) == 4, "sizeof(float) must be 32 bits.");
		write_float<float>(value);
	}

	template<typename SourceType>
	void write_int64(SourceType value) {
		write_int<std::uint64_t>(value);
	}

	template<typename SourceType>
	void write_int32(SourceType value) {
		write_int<std::uint32_t>(value);
	}

	template<typename SourceType>
	void write_int16(SourceType value) {
		write_int<std::uint16_t>(value);
	}

	template<typename SourceType>
	void write_int8(SourceType value) {
		write_int<std::uint8_t>(value);
	}

	template<typename DestinationType = double>
	DestinationType read_float64() {
		static_assert(sizeof(double) == 8, "sizeof(double) must be 64 bits.");
		return read_float<double, DestinationType>();
	}

	template<typename DestinationType = float>
	DestinationType read_float32() {
		static_assert(sizeof(float) == 4, "sizeof(float) must be 32 bits.");
		return read_float<float, DestinationType>();
	}

	template<typename DestinationType = std::int64_t>
	DestinationType read_int64() {
		return read_int<std::uint64_t, DestinationType>();
	}

	template<typename DestinationType = std::int32_t>
	DestinationType read_int32() {
		return read_int<std::uint32_t, DestinationType>();
	}

	template<typename DestinationType = std::int16_t>
	DestinationType read_int16() {
		return read_int<std::uint16_t, DestinationType>();
	}

	template<typename DestinationType = std::int8_t>
	DestinationType read_int8() {
		return read_int<std::uint8_t, DestinationType>();
	}

	template<typename SourceType, typename DestinationType = SourceType>
	std::optional<DestinationType> read_optional_int() {
		static_assert(std::is_integral_v<SourceType>, "SourceType must be integral.");
		return read_optional<SourceType, DestinationType>();
	}
	
	template<typename DestinationType = std::int64_t>
	DestinationType read_optional_int64() {
		return read_optional_int<std::uint64_t, DestinationType>();
	}

	template<typename DestinationType = std::int32_t>
	std::optional<DestinationType> read_optional_int32() {
		return read_optional_int<std::uint32_t, DestinationType>();
	}

	template<typename DestinationType = std::int16_t>
	std::optional<DestinationType> read_optional_int16() {
		return read_optional_int<std::uint16_t, DestinationType>();
	}

	template<typename DestinationType = std::int8_t>
	std::optional<DestinationType> read_optional_int8() {
		return read_optional_int<std::uint8_t, DestinationType>();
	}

	template<typename SourceType, typename DestinationType = SourceType>
	std::optional<DestinationType> read_optional_float() {
		static_assert(std::is_floating_point_v<SourceType>, "SourceType must be floating point.");
		return read_trivial<SourceType, DestinationType>();
	}

	template<typename DestinationType = double>
	std::optional<DestinationType> read_optional_float64() {
		return read_optional_float<double, DestinationType>();
	}

	template<typename DestinationType = float>
	std::optional<DestinationType> read_optional_float32() {
		return read_optional_float<float, DestinationType>();
	}

	template<size_length Size = size_length::four_bytes>
	void write_size(size_type value) {
		switch (Size) {
		case size_length::one_byte:
			write_int8(value);
			break;
		case size_length::two_bytes:
			write_int16(value);
			break;
		case size_length::four_bytes:
			write_int32(value);
			break;
		case size_length::eight_bytes:
			write_int64(value);
			break;
		}
	}

	template<size_length Size = size_length::four_bytes>
	size_type read_size() {
		switch (Size) {
		case size_length::one_byte: return static_cast<size_type>(read_int8());
		case size_length::two_bytes: return static_cast<size_type>(read_int16());
		case size_length::four_bytes: return static_cast<size_type>(read_int32());
		case size_length::eight_bytes: return static_cast<size_type>(read_int64());
		}
	}

	template<typename Type>
	void write_struct(Type value) {
		write_trivial<Type, Type>(value);
	}

	template<typename Type>
	Type read_struct() {
		return read_trivial<Type>();
	}

	void write_bool(bool value);
	bool read_bool();

	void write_string_array(const std::vector<std::string>& values);
	std::vector<std::string> read_string_array();

	[[nodiscard]] size_type read_line(char* destination, size_type max_size, bool remove_newline);
	[[nodiscard]] std::string read_line(bool remove_newline);

	[[nodiscard]] size_type find_first(std::string_view key, size_type start = 0) const;
	[[nodiscard]] size_type find_last(std::string_view key, size_type start = std::string_view::npos) const;

	char* data() const;
	bool is_owner() const;
	void reset();

private:
	
	char* begin{ nullptr };
	char* end{ nullptr };
	char* read_position{ nullptr }; // todo: store as uint32 is probably better
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
void write_file(const std::filesystem::path& path, const char* source, std::uintmax_t size);
void write_file(const std::filesystem::path& path, io_stream& source);

void append_file(const std::filesystem::path& path, std::string_view source);

std::string read_file(const std::filesystem::path& path);
void read_file(const std::filesystem::path& path, io_stream& destination);
io_stream file_section_stream(const std::filesystem::path& path, std::uintmax_t offset, std::uintmax_t max_size);

}

NFWK_STDSPEC_FORMATTER(nfwk::entry_inclusion);
NFWK_STDSPEC_FORMATTER(nfwk::size_length);
