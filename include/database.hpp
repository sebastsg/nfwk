#pragma once

#include "log.hpp"
#include "assert.hpp"

#include <filesystem>

namespace nfwk {

class database_query_result {
public:

	friend class database_connection;
	
	enum class result_status { success, error };

	database_query_result(const database_query_result&) = delete;
	database_query_result(database_query_result&&) = delete;

	~database_query_result();

	database_query_result& operator=(const database_query_result&) = delete;
	database_query_result& operator=(database_query_result&&) = delete;

	operator bool() const;

	bool is_empty() const;
	int count() const;
	int columns() const;

	template<typename T>
	T get(int row, int column) const {
		const value_data value{ get_raw(row, column) };
		return nfwk::from_string<T>({ value.data, value.size }).value_or(T{});
	}
	
	std::string_view get_string(int row, int column) const;
	std::string get_string_copy(int row, int column) const;
	bool is_null(int row, int column) const;

	std::string get_error_message() const;
	result_status get_status() const;

private:

	class internal_query_result;
	
	database_query_result(internal_query_result* result, std::string_view query);

	struct value_data {
		const char* const data;
		const std::size_t size;
		value_data(const char* data, std::size_t size) : data{ data }, size{ size } {}
	};

	value_data get_raw(int row, int column) const;
	
	internal_query_result* result{ nullptr };
	int rows{ 0 };

};

// todo: check host endianness
template<typename T32>
T32 host_to_network_32(T32 host) {
	static_assert(sizeof(T32) == 4);
	const auto* bytes = reinterpret_cast<const std::uint8_t*>(&host);
	return
		static_cast<T32>(bytes[0]) << 24 |
		static_cast<T32>(bytes[1]) << 16 |
		static_cast<T32>(bytes[2]) << 8 |
		static_cast<T32>(bytes[3]);
}

template<typename T64>
T64 host_to_network_64(T64 host) {
	static_assert(sizeof(T64) == 8);
	const auto* bytes = reinterpret_cast<const std::uint8_t*>(&host);
	return
		static_cast<T64>(bytes[0]) << 56 |
		static_cast<T64>(bytes[1]) << 48 |
		static_cast<T64>(bytes[2]) << 40 |
		static_cast<T64>(bytes[3]) << 32 |
		static_cast<T64>(bytes[4]) << 24 |
		static_cast<T64>(bytes[5]) << 16 |
		static_cast<T64>(bytes[6]) << 8 | 
		static_cast<T64>(bytes[7]);
}

template<typename T>
T host_to_network(T host) {
	if constexpr (sizeof(T) == 4) {
		return host_to_network_32(host);
	} else if constexpr (sizeof(T) == 8) {
		return host_to_network_64(host);
	}
}

class database_cstring {
public:

	std::string_view string;

	database_cstring(std::string_view string) : string{ string } {
		// This can result in access violation if there is no null.
		// However, that would happen afterwards regardless, so it's better to crash here.
		ASSERT(*(string.data() + string.size()) == '\0');
	}

};

template<typename... OriginalParameters>
class query_parameter_list {
public:

	static constexpr auto count = sizeof...(OriginalParameters);

	const char* values[count + 1];
	int lengths[count + 1];
	int formats[count + 1];

	query_parameter_list(const OriginalParameters&... parameters) {
		extract(parameters...);
	}

	template<typename... Parameters>
	void extract(std::string_view value, const Parameters&... parameters) {
		values[index] = value.data();
		lengths[index] = value.size();
		formats[index] = 1;
		index++;
		extract(parameters...);
	}

	template<typename... Parameters>
	void extract(database_cstring value, const Parameters&... parameters) {
		values[index] = value.string.data();
		lengths[index] = 0;
		formats[index] = 0;
		index++;
		extract(parameters...);
	}

	template<typename... Parameters>
	void extract(std::int32_t value, const Parameters&... parameters) {
		buffer64[index] = host_to_network_32(value);
		values[index] = reinterpret_cast<const char*>(&buffer64[index]);
		lengths[index] = sizeof(std::int32_t);
		formats[index] = 1;
		index++;
		extract(parameters...);
	}

	template<typename... Parameters>
	void extract(std::int64_t value, const Parameters&... parameters) {
		buffer64[index] = host_to_network_64(value);
		values[index] = reinterpret_cast<const char*>(&buffer64[index]);
		lengths[index] = sizeof(std::int64_t);
		formats[index] = 1;
		index++;
		extract(parameters...);
	}

	template<typename... Parameters>
	void extract(double value, const Parameters&... parameters) {
		static_assert(sizeof(std::int64_t) >= sizeof(double));
		buffer64[index] = *reinterpret_cast<const std::int64_t*>(&value);
		buffer64[index] = host_to_network_64(buffer64[index]);
		values[index] = reinterpret_cast<const char*>(&buffer64[index]);
		lengths[index] = sizeof(double);
		formats[index] = 1;
		index++;
		extract(parameters...);
	}

private:

#if 0
	template<typename T, bool ConvertToNetwork = true>
	void extract_raw(T value) {
		static_assert(sizeof(std::uint64_t) >= sizeof(T));
		if constexpr (ConvertToNetwork) {
			const auto network_value = host_to_network(value);
			std::memcpy(&buffer_64[index], &network_value, sizeof(T));
		} else {
			std::memcpy(&buffer_64[index], &value, sizeof(T));
		}
		values[index] = reinterpret_cast<const char*>(&buffer_64[index]);
		lengths[index] = sizeof(T);
		formats[index] = 1;
		index++;
	}
#endif
	
	void extract() {}
	
	std::uint64_t buffer64[count + 1]{};
	int index{ 0 };
	
};

class database_connection {
public:

	database_connection(std::string_view host, int port, std::string_view name, std::string_view username, std::string_view password);
	database_connection(const database_connection&) = delete;
	database_connection(database_connection&&) = delete;

	~database_connection();

	database_connection& operator=(const database_connection&) = delete;
	database_connection& operator=(database_connection&&) = delete;

	bool is_connected() const;

	template<typename... Parameters>
	[[maybe_unused]] database_query_result execute(std::string_view query, const Parameters&... parameters) const {
		query_parameter_list list{ parameters... };
		return execute_sql(query, list.values, list.lengths, list.formats, list.count);
	}

	template<typename... Parameters>
	[[maybe_unused]] database_query_result call_procedure(std::string procedure, const Parameters&... parameters) const {
		query_parameter_list list{ parameters... };
		std::string query{ procedure + "( " }; // the space is important, see below
		for (int i{ 1 }; i <= list.count; i++) {
			query += "$" + std::to_string(i) + ',';
		}
		query.back() = ')';
		return execute_sql(query, list.values, list.lengths, list.formats, list.count);
	}

	template<typename... Parameters>
	[[maybe_unused]] std::int64_t insert_row(std::string_view table, bool has_serial_id, const Parameters&... parameters) {
		query_parameter_list list{ parameters... };
		std::string query{ "insert into \"" + std::string{ table } + "\" values ( " };
		if (has_serial_id) {
			query += "default,";
		}
		for (int i{ 1 }; i <= list.count; i++) {
			query += "$" + std::to_string(i) + ',';
		}
		query.back() = ')';
		if (has_serial_id) {
			query += " returning id";
		}
		if (const auto result = execute_sql(query, list.values, list.lengths, list.formats, list.count)) {
			return result.get<std::int64_t>(0, 0);
		} else {
			return {};
		}
	}

	void execute_sql_file(const std::filesystem::path& path, bool split);

private:
	
	database_query_result execute_sql(std::string_view query, const char** params, const int* lengths, const int* formats, int count) const;

	class internal_connection;

	internal_connection* session{ nullptr };

};

}
