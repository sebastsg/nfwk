#pragma once

#include "io.hpp"

#include <unordered_map>
#include <functional>

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

	variable_type type = variable_type::string;
	std::string name;
	std::string value;
	bool is_persistent = true;

	variable() = default;
	variable(variable_type type, std::string name, std::string value, bool persistent);

	bool compare(const std::string& right, variable_comparison comp_operator) const;
	void modify(const std::string& value, variable_modification mod_operator);

};

class variable_map {
public:

	variable* global(const std::string& name);
	variable* local(int scope_id, const std::string& name);

	void create_global(variable var);
	void create_local(int scope_id, variable var);

	void delete_global(const std::string& name);
	void delete_local(int scope_id, const std::string& name);

	void for_each_global(const std::function<void(const variable&)>& function) const;
	void for_each_local(const std::function<void(int, const variable&)>& function) const;

	void write(io_stream& stream) const;
	void read(io_stream& stream);

private:

	std::vector<variable> globals;
	std::unordered_map<int, std::vector<variable>> locals;

};

}
