#pragma once

#include "io.hpp"

#include <functional>
#include <optional>
#include <variant>

namespace no {

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

class variable {
public:

	std::string name;
	bool persistent{ true };
	variable_type type{ variable_type::string };
	std::variant<std::string, int, float> value;
	
	variable() = default;
	variable(io_stream& stream);

	bool compare(const std::string& right, variable_comparison comparison_operator) const;
	bool compare(int right, variable_comparison comparison_operator) const;
	bool compare(float right, variable_comparison comparison_operator) const;
	bool compare(const variable& right, variable_comparison comparison_operator) const;

	void modify(const std::string& right, variable_operator mod_operator);
	void modify(int right, variable_operator mod_operator);
	void modify(float right, variable_operator mod_operator);
	void modify(const variable& right, variable_operator mod_operator);

	void write(io_stream& stream) const;
	void read(io_stream& stream);

};

class variable_scope {
public:

	variable_scope() = default;
	variable_scope(std::optional<int> id);
	variable_scope(io_stream& stream);

	std::optional<int> id() const;
	std::optional<std::reference_wrapper<variable>> find(const std::string& name);
	void add(variable variable);
	void remove(const std::string& name);
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

	std::optional<std::reference_wrapper<variable>> find(std::optional<int> scope, const std::string& name);
	void add(std::optional<int> scope, variable variable);
	void remove(std::optional<int> scope, const std::string& name);
	void for_each(std::optional<int> scope, const std::function<void(const variable&)>& function) const;

	void write(io_stream& stream) const;
	void read(io_stream& stream);
	
private:

	variable_scope* find_scope(std::optional<int> scope);
	const variable_scope* find_scope(std::optional<int> scope) const;

	std::vector<variable_scope> scopes;
	variable_scope global_scope;

};

}

std::ostream& operator<<(std::ostream& out, no::variable_comparison type);
std::ostream& operator<<(std::ostream& out, no::variable_type type);
std::ostream& operator<<(std::ostream& out, no::variable_operator type);
