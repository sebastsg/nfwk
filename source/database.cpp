#include "database.hpp"
#include "utility_functions.hpp"

#include "postgresql/libpq-fe.h"

namespace nfwk {

database_query_result::database_query_result(internal_query_result* result_, std::string_view query) : result{ result_ } {
	if (get_status() == result_status::error) {
		error(core::log, "Query: {}.\nError message: {}", query, get_error_message());
	}
	rows = PQntuples(reinterpret_cast<PGresult*>(result));
}

database_query_result::~database_query_result() {
	PQclear(reinterpret_cast<PGresult*>(result));
}

database_query_result::operator bool() const {
	return result && rows > 0;
}

bool database_query_result::is_empty() const {
	return rows == 0;
}

int database_query_result::count() const {
	return rows;
}

int database_query_result::columns() const {
	return PQnfields(reinterpret_cast<PGresult*>(result));
}

database_query_result::value_data database_query_result::get_raw(int row, int column) const {
	const auto* result_ = reinterpret_cast<const PGresult*>(result);
	const auto* data = PQgetvalue(result_, row, column);
	const auto size = static_cast<std::size_t>(PQgetlength(result_, row, column));
	return { data, size };
}

std::string_view database_query_result::get_string(int row, int column) const {
	const auto* result_ = reinterpret_cast<const PGresult*>(result);
	const auto* data = PQgetvalue(result_, row, column);
	const auto size = static_cast<std::size_t>(PQgetlength(result_, row, column));
	return { reinterpret_cast<const char*>(data), size };
}

std::string database_query_result::get_string_copy(int row, int column) const {
	const auto* result_ = reinterpret_cast<const PGresult*>(result);
	const auto* data = PQgetvalue(result_, row, column);
	const auto size = static_cast<std::size_t>(PQgetlength(result_, row, column));
	return { reinterpret_cast<const char*>(data), size };
}

bool database_query_result::is_null(int row, int column) const {
	return PQgetisnull(reinterpret_cast<const PGresult*>(result), row, column) != 0;
}

std::string database_query_result::get_error_message() const {
	auto message = PQresultVerboseErrorMessage(reinterpret_cast<PGresult*>(result), PQERRORS_VERBOSE, PQSHOW_CONTEXT_ALWAYS);
	std::string copied_message{ reinterpret_cast<const char*>(message) };
	PQfreemem(message);
	return copied_message;
}

database_query_result::result_status database_query_result::get_status() const {
	switch (PQresultStatus(reinterpret_cast<PGresult*>(result))) {
	case PGRES_BAD_RESPONSE:
	case PGRES_FATAL_ERROR:
	case PGRES_NONFATAL_ERROR:
		return result_status::error;
	default:
		return result_status::success;
	}
}

database_connection::database_connection(std::string_view host, int port, std::string_view name, std::string_view username, std::string_view password) {
	info(core::log, "Connecting to database");
	const auto options = format("host={} port={} dbname={} user={} password={}", host, port, name, username, password);
	auto session_ = PQconnectdb(reinterpret_cast<const char*>(options.c_str()));
	if (PQstatus(session_) != CONNECTION_OK) {
		error(core::log, "Failed to connect to database: {}", PQerrorMessage(session_));
	}
	session = reinterpret_cast<internal_connection*>(session_);
}

database_connection::~database_connection() {
	PQfinish(reinterpret_cast<PGconn*>(session));
}

bool database_connection::is_connected() const {
	return session != nullptr;
}

void database_connection::execute_sql_file(const std::filesystem::path& path, bool split) {
	const auto& sql = read_file(path);
	if (sql.empty()) {
		error(core::log, "Unable to read file: {}", path);
		return;
	}
	info(core::log, "Executing sql file: {} (split: {})", path, split ? "yes" : "no");
	if (split) {
		for (const auto& statement : split_string(sql, ';')) {
			execute_sql(statement, nullptr, nullptr, nullptr, 0);
		}
	} else {
		execute_sql(sql, nullptr, nullptr, nullptr, 0);
	}
}

database_query_result database_connection::execute_sql(std::string_view query, const char** params, const int* lengths, const int* formats, int count) const {
	constexpr int result_format{ 0 }; // text result
	auto result = PQexecParams(reinterpret_cast<PGconn*>(session), reinterpret_cast<const char*>(query.data()), count, nullptr, params, lengths, formats, result_format);
	return { reinterpret_cast<database_query_result::internal_query_result*>(result), query };
}

}
