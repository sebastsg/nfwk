#pragma once

#include "io.hpp"

#include <functional>
#include <optional>
#include <variant>

namespace nfwk::script {

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
	std::variant<std::string, int, float> value;
	variable_type type{ variable_type::string };
	bool persistent{ true };
	
	variable() = default;
	variable(io_stream& stream);

	bool compare(std::string_view right, variable_comparison comparison_operator) const;
	bool compare(int right, variable_comparison comparison_operator) const;
	bool compare(float right, variable_comparison comparison_operator) const;
	bool compare(const variable& right, variable_comparison comparison_operator) const;

	void modify(const std::string& right, variable_operator modify_operator);
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
	variable_scope(const variable_scope&) = delete;
	variable_scope(variable_scope&&) = delete;

	~variable_scope() = default;

	variable_scope& operator=(const variable_scope&) = delete;
	variable_scope& operator=(variable_scope&&) = delete;
	
	std::optional<int> id() const;
	variable* find(std::string_view name);
	void add(variable variable);
	void remove(std::string_view name);
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

	~variable_registry() = default;

	variable_registry& operator=(const variable_registry&) = delete;
	variable_registry& operator=(variable_registry&&) = delete;

	variable* find(std::optional<int> scope_id, std::string_view name);
	void add(std::optional<int> scope_id, variable variable);
	void remove(std::optional<int> scope_id, const std::string& name);
	void for_each(std::optional<int> scope_id, const std::function<void(const variable&)>& function) const;

	void write(io_stream& stream) const;
	void read(io_stream& stream);
	
private:

	std::shared_ptr<variable_scope> find_scope(std::optional<int> scope_id) const;

	std::vector<std::shared_ptr<variable_scope>> scopes;
	std::shared_ptr<variable_scope> global_scope;

};

}

std::ostream& operator<<(std::ostream& out, nfwk::script::variable_comparison type);
std::ostream& operator<<(std::ostream& out, nfwk::script::variable_type type);
std::ostream& operator<<(std::ostream& out, nfwk::script::variable_operator type);

NFWK_STDSPEC_FORMATTER(nfwk::script::variable_comparison);
NFWK_STDSPEC_FORMATTER(nfwk::script::variable_type);
NFWK_STDSPEC_FORMATTER(nfwk::script::variable_operator);
