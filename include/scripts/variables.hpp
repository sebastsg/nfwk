#pragma once

#include "io.hpp"

#include <functional>
#include <optional>
#include <variant>

namespace nfwk {

enum class other_variable_type { value, local, global };

enum class variable_comparison {
	equal,
	not_equal,
	greater_than,
	less_than,
	equal_or_greater_than,
	equal_or_less_than
};

enum class variable_type { string, integer, boolean, floating };
enum class variable_operator { set, negate, add, multiply, divide };

inline std::u8string to_string(variable_comparison type) {
	switch (type) {
	case variable_comparison::equal: return u8"= Equals";
	case variable_comparison::not_equal: return u8"!= Not Equals";
	case variable_comparison::greater_than: return u8"> Greater Than";
	case variable_comparison::less_than: return u8"< Less Than";
	case variable_comparison::equal_or_greater_than: return u8">= Equal or Greater Than";
	case variable_comparison::equal_or_less_than: return u8"<= Equal or Less Than";
	}
}

inline std::u8string to_string(variable_type type) {
	switch (type) {
	case variable_type::string: return u8"String";
	case variable_type::integer: return u8"Integer";
	case variable_type::boolean: return u8"Boolean";
	case variable_type::floating: return u8"Float";
	}
}

inline std::u8string to_string(variable_operator type) {
	switch (type) {
	case variable_operator::set: return u8"Set";
	case variable_operator::negate: return u8"Negate";
	case variable_operator::add: return u8"Add";
	case variable_operator::multiply: return u8"Multiply";
	case variable_operator::divide: return u8"Divide";
	}
}

class variable {
public:

	std::u8string name;
	bool persistent{ true };
	variable_type type{ variable_type::string };
	std::variant<std::u8string, int, float> value;
	
	variable() = default;
	variable(io_stream& stream);

	bool compare(std::u8string_view right, variable_comparison comparison_operator) const;
	bool compare(int right, variable_comparison comparison_operator) const;
	bool compare(float right, variable_comparison comparison_operator) const;
	bool compare(const variable& right, variable_comparison comparison_operator) const;

	void modify(const std::u8string& right, variable_operator modify_operator);
	void modify(int right, variable_operator modify_operator);
	void modify(float right, variable_operator modify_operator);
	void modify(const variable& right, variable_operator modify_operator);

	void write(io_stream& stream) const;
	void read(io_stream& stream);

};

class variable_scope {
public:

	variable_scope() = default;
	variable_scope(std::optional<int> id);
	variable_scope(io_stream& stream);

	std::optional<int> id() const;
	variable* find(std::u8string_view name);
	void add(variable variable);
	void remove(std::u8string_view name);
	void for_each(const std::function<void(const variable&)>& function) const;

	void write(io_stream& stream) const;
	void read(io_stream& stream);

private:

	std::vector<variable> variables;
	std::optional<int> scope_id;

};

class variable_registry {
public:

	variable_registry() = default;
	variable_registry(const variable_registry&) = delete;
	variable_registry(variable_registry&&) = delete;

	variable_registry& operator=(const variable_registry&) = delete;
	variable_registry& operator=(variable_registry&&) = delete;

	variable* find(std::optional<int> scope_id, std::u8string_view name);
	void add(std::optional<int> scope_id, variable variable);
	void remove(std::optional<int> scope_id, const std::u8string& name);
	void for_each(std::optional<int> scope_id, const std::function<void(const variable&)>& function) const;

	void write(io_stream& stream) const;
	void read(io_stream& stream);
	
private:

	variable_scope* find_scope(std::optional<int> scope_id);
	const variable_scope* find_scope(std::optional<int> scope_id) const;

	std::vector<variable_scope> scopes;
	variable_scope global_scope;

};

}
