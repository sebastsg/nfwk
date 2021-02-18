export module nfwk.core:io_stream;

import std.core;
import std.filesystem;
import :files;

export namespace nfwk {

class io_stream;

void write_file(const std::filesystem::path& path, io_stream& source);
void read_file(const std::filesystem::path& path, io_stream& stream);

class io_stream {
public:

	enum class construct_by { copy, move, shallow_copy };

	io_stream() = default;

	io_stream(size_t size) {
		allocate(size);
	}

	io_stream(char* data, std::size_t size, construct_by construction) {
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

	io_stream(const std::filesystem::path& path) {
		read_file(path, *this);
	}

	io_stream(const io_stream&) = delete;

	io_stream(io_stream&& that) noexcept {
		std::swap(begin, that.begin);
		std::swap(end, that.end);
		std::swap(read_position, that.read_position);
		std::swap(write_position, that.write_position);
		std::swap(owner, that.owner);
	}

	~io_stream() {
		free();
	}

	io_stream& operator=(const io_stream&) = delete;

	io_stream& operator=(io_stream&& that) noexcept {
		std::swap(begin, that.begin);
		std::swap(end, that.end);
		std::swap(read_position, that.read_position);
		std::swap(write_position, that.write_position);
		std::swap(owner, that.owner);
		return *this;
	}

	void allocate(std::size_t size) {
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

	void resize(std::size_t new_size) {
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

	void resize_if_needed(std::size_t size_to_write) {
		if (size_to_write > size_left_to_write()) {
			// size() might be 0, so make sure the data fits comfortably.
			const std::size_t new_size{ size() * 2 + size_to_write + 64 };
			resize(new_size);
		}
	}

	void free() {
		if (owner) {
			delete[] begin;
		}
		begin = nullptr;
		end = nullptr;
		read_position = nullptr;
		write_position = nullptr;
	}

	// move everything from the read position to the beginning
	// useful when we have extracted some data, and want to read the rest with a read index of 0
	void shift_read_to_begin() {
		const auto shift_size = static_cast<std::size_t>(write_position - read_position);
		std::memcpy(begin, read_position, shift_size); // copy read-to-write to begin-to-size.
		read_position = begin;
		write_position = begin + shift_size;
	}

	void set_read_index(std::size_t index) {
		if (index >= size()) {
			read_position = end;
		} else {
			read_position = begin + index;
		}
	}

	void set_write_index(std::size_t index) {
		if (index >= size()) {
			write_position = end;
		} else {
			write_position = begin + index;
		}
	}

	void move_read_index(long long size) {
		const auto index = static_cast<long long>(read_index()) + size;
		set_read_index(static_cast<std::size_t>(index));
	}

	void move_write_index(long long size) {
		const auto index = static_cast<long long>(write_index()) + size;
		set_write_index(static_cast<std::size_t>(index));
	}

	bool empty() const {
		return begin == end;
	}

	std::size_t size() const {
		return static_cast<std::size_t>(end - begin);
	}

	std::size_t size_left_to_write() const {
		return static_cast<std::size_t>(end - write_position);
	}

	std::size_t size_left_to_read() const {
		return static_cast<std::size_t>(write_position - read_position);
	}

	std::size_t read_index() const {
		return static_cast<std::size_t>(read_position - begin);
	}

	std::size_t write_index() const {
		return static_cast<std::size_t>(write_position - begin);
	}

	char* at(std::size_t index) const {
		return begin + index;
	}

	char* at_read() const {
		return read_position;
	}

	char* at_write() const {
		return write_position;
	}

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
		if (read_position + sizeof(T) > end) {
			return {};
		}
		T value;
		std::memcpy(&value, read_position, sizeof(T));
		read_position += sizeof(T);
		return value;
	}

	template<>
	std::string read() {
		std::size_t length = read<std::uint32_t>();
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
		std::memcpy(write_position, &value, sizeof(T));
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

	std::size_t read_line(char* destination, std::size_t max_size, bool remove_newline) {
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
		std::memcpy(destination, read_position, i);
		read_position += end_of_the_line;
		destination[i] = '\0';
		return i;
	}

	std::string read_line(bool remove_newline) {
		std::string result;
		char buffer[256];
		while (read_line(buffer, 256, remove_newline) != 0) {
			result += buffer;
		}
		return result;
	}

	int find_first(const std::string& key, std::size_t start) const {
		auto found = std::strstr(begin + start, key.c_str());
		return found ? static_cast<int>(found - begin) : -1;
	}

	int find_last(const std::string& key, std::size_t start) const {
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

	char* data() const {
		return begin;
	}

	bool is_owner() const {
		return owner;
	}

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

void write_file(const std::filesystem::path& path, io_stream& source) {
	//write_file(path, source.data(), source.write_index());
}

void read_file(const std::filesystem::path& path, io_stream& stream) {
	/*if (std::ifstream file{ path, std::ios::binary }; file.is_open()) {
		std::stringstream result;
		result << file.rdbuf();
		const auto& string = result.str();
		stream.write(string.c_str(), string.size());
	}*/
}

}