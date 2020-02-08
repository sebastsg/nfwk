#pragma once

#include "io.hpp"

#include <functional>
#include <optional>

namespace no {

enum class variable_comparison {
	equal,
	not_equal,
	greater_than,
	less_than,
	equal_or_greater_than,
	equal_or_less_than
};

enum class variable_type { string, integer, floating, boolean };
enum class variable_modification { set, negate, add, multiply, divide };

struct variable {

	variable_type type{ variable_type::string };
	std::string name;
	std::string value;
	bool persistent{ true };

	variable() = default;
	variable(variable_type type, std::string name, std::string value, bool persistent);

	bool compare(const std::string& right, variable_comparison comp_operator) const;
	void modify(const std::string& value, variable_modification mod_operator);

};

class variable_scope {
public:

	int id() const;

	variable* find(const std::string& name);
	void add(variable variable);
	void remove(const std::string& name);
	void for_each(const std::function<void(const variable&)>& function) const;

	void write(io_stream& stream) const;
	void read(io_stream& stream);

private:

	std::vector<variable> variables;
	int scope_id{ 0 };

};

class variable_registry {
public:

	variable* find(std::optional<int> scope, const std::string& name);
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
