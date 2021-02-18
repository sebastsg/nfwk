export module nfwk.scripts:variables;

import std.core;
import nfwk.core;
import nfwk.assets;

export namespace nfwk {

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

	variable(io_stream& stream) {
		read(stream);
	}

	bool compare(const std::string& right, variable_comparison comparison_operator) const {
		const auto string = std::get<std::string>(value);
		switch (comparison_operator) {
		case variable_comparison::equal: return string == right;
		case variable_comparison::not_equal: return string != right;
		case variable_comparison::greater_than: return string.size() > right.size();
		case variable_comparison::less_than: return string.size() < right.size();
		case variable_comparison::equal_or_greater_than: return string == right || string.size() > right.size();
		case variable_comparison::equal_or_less_than: return string == right || string.size() < right.size();
		default: return false;
		}
	}

	bool compare(int right, variable_comparison comparison_operator) const {
		const auto integer = std::get<int>(value);
		switch (comparison_operator) {
		case variable_comparison::equal: return integer == right;
		case variable_comparison::not_equal: return integer != right;
		case variable_comparison::greater_than: return integer > right;
		case variable_comparison::less_than: return integer < right;
		case variable_comparison::equal_or_greater_than: return integer >= right;
		case variable_comparison::equal_or_less_than: return integer <= right;
		default: return false;
		}
	}

	bool compare(float right, variable_comparison comparison_operator) const {
		const auto floating = std::get<float>(value);
		switch (comparison_operator) {
		case variable_comparison::equal: return floating == right;
		case variable_comparison::not_equal: return floating != right;
		case variable_comparison::greater_than: return floating > right;
		case variable_comparison::less_than: return floating < right;
		case variable_comparison::equal_or_greater_than: return floating >= right;
		case variable_comparison::equal_or_less_than: return floating <= right;
		default: return false;
		}
	}

	bool compare(const variable& right, variable_comparison comparison_operator) const {
		switch (right.type) {
		case variable_type::string: return compare(std::get<std::string>(right.value), comparison_operator);
		case variable_type::integer:
		case variable_type::boolean: return compare(std::get<int>(right.value), comparison_operator);
		case variable_type::floating: return compare(std::get<float>(right.value), comparison_operator);
		default: return false;
		}
	}

	void modify(const std::string& right, variable_operator modify_operator) {
		switch (modify_operator) {
		case variable_operator::set:
			value = right;
			break;
		case variable_operator::add:
			std::get<std::string>(value) += right;
			break;
		default:
			warning("scripts", "Cannot run operator {} on a string - {}", modify_operator, name);
			break;
		}
	}

	void modify(int right, variable_operator modify_operator) {
		switch (modify_operator) {
		case variable_operator::set:
			value = right;
			break;
		case variable_operator::negate:
			std::get<int>(value) = !std::get<int>(value);
			break;
		case variable_operator::add:
			std::get<int>(value) += right;
			break;
		case variable_operator::multiply:
			std::get<int>(value) *= right;
			break;
		case variable_operator::divide:
			if (right != 0) {
				std::get<int>(value) /= right;
			} else {
				warning("scripts", "Prevented division of {} by 0. Setting the value to 0 to minimize errors.", name);
				std::get<int>(value) = 0;
			}
			break;
		}
	}

	void modify(float right, variable_operator modify_operator) {
		switch (modify_operator) {
		case variable_operator::set:
			std::get<float>(value) = right;
			break;
		case variable_operator::negate:
			std::get<float>(value) = !std::get<float>(value);
			break;
		case variable_operator::add:
			std::get<float>(value) += right;
			break;
		case variable_operator::multiply:
			std::get<float>(value) *= right;
			break;
		case variable_operator::divide:
			if (right != 0.0f) {
				std::get<float>(value) /= right;
			} else {
				warning("scripts", "Prevented division of {} by 0.0. Setting the value to 0.0 to minimize errors.", name);
				std::get<float>(value) = 0.0f;
			}
			break;
		}
	}

	void modify(const variable& right, variable_operator modify_operator) {
		switch (right.type) {
		case variable_type::string:
			modify(std::get<std::string>(right.value), modify_operator);
			break;
		case variable_type::integer:
		case variable_type::boolean:
			modify(std::get<int>(right.value), modify_operator);
			break;
		case variable_type::floating:
			modify(std::get<float>(right.value), modify_operator);
			break;
		}
	}

	std::variant<std::string, int, float> default_value() const {
		switch (type) {
		case variable_type::string: return "";
		case variable_type::integer:
		case variable_type::boolean: return 0;
		case variable_type::floating: return 0.0f;
		default:
			warning("scripts", "Invalid variable type.");
			return 0;
		}
	}

	void write(io_stream& stream) const {
		stream.write(static_cast<std::int32_t>(type));
		stream.write(name);
		switch (type) {
		case variable_type::string:
			stream.write(std::get<std::string>(value));
			break;
		case variable_type::integer:
			stream.write<std::int32_t>(std::get<int>(value));
			break;
		case variable_type::boolean:
			stream.write(static_cast<bool>(std::get<int>(value)));
			break;
		case variable_type::floating:
			stream.write(std::get<float>(value));
			break;
		}
		stream.write(persistent);
	}

	void read(io_stream& stream) {
		type = static_cast<variable_type>(stream.read<std::int32_t>());
		name = stream.read<std::string>();
		switch (type) {
		case variable_type::string:
			value = stream.read<std::string>();
			break;
		case variable_type::integer:
			value = static_cast<int>(stream.read<std::int32_t>());
			break;
		case variable_type::boolean:
			value = static_cast<int>(stream.read<bool>());
			break;
		case variable_type::floating:
			value = stream.read<float>();
			break;
		}
		persistent = stream.read<bool>();
	}

};

class variable_scope {
public:

	variable_scope() = default;

	variable_scope(std::optional<int> id) : scope_id{ id } {}

	variable_scope(io_stream& stream) {
		read(stream);
	}

	std::optional<int> id() const {
		return scope_id;
	}

	std::optional<std::reference_wrapper<variable>> find(const std::string& name) {
		for (auto& variable : variables) {
			if (variable.name == name) {
				return variable;
			}
		}
		return std::nullopt;
	}

	void add(variable variable, bool replace) {
		auto existing = find(variable.name);
		if (!existing) {
			variables.emplace_back(variable);
		} else if (existing && replace) {
			existing.value() = variable;
		} else {
			warning("scripts", "Variable {} already exists.", variable.name);
		}
	}

	void remove(const std::string& name) {
		for (size_t i{ 0 }; i < variables.size(); i++) {
			if (variables[i].name == name) {
				variables.erase(variables.begin() + i);
				break;
			}
		}
	}

	void for_each(const std::function<void(const variable&)>& function) const {
		for (const auto& variable : variables) {
			function(variable);
		}
	}

	void write(io_stream& stream) const {
		stream.write_optional<std::int32_t>(scope_id);
		stream.write(static_cast<std::int32_t>(variables.size()));
		for (const auto& variable : variables) {
			variable.write(stream);
		}
	}

	void read(io_stream& stream) {
		scope_id = stream.read_optional<std::int32_t>();
		const auto count{ stream.read<std::int32_t>() };
		for (std::int32_t i{ 0 }; i < count; i++) {
			variables.emplace_back(stream);
		}
	}

private:

	std::vector<variable> variables;
	std::optional<int> scope_id;

};

}

namespace nfwk {

static std::vector<variable_scope> scopes;
static variable_scope global_scope;

static variable_scope* find_scope(std::optional<int> scope_id) {
	if (scope_id.has_value()) {
		for (auto& scope : scopes) {
			if (scope.id() == scope_id.value()) {
				return &scope;
			}
		}
		return nullptr;
	} else {
		return &global_scope;
	}
}

static void write_variables(io_stream& stream) {
	stream.write(static_cast<std::int32_t>(scopes.size()));
	for (const auto& scope : scopes) {
		scope.write(stream);
	}
}

static void read_variables(io_stream& stream) {
	const auto count = stream.read<std::int32_t>();
	for (std::int32_t i{ 0 }; i < count; i++) {
		scopes.emplace_back(stream);
	}
}

}

export namespace nfwk {

void save_variables() {
	io_stream stream;
	write_variables(stream);
	//write_file(asset_path("variables.nfwk-data"), stream);
}

void load_variables() {
	//if (io_stream stream{ asset_path("variables.nfwk-data") }; !stream.empty()) {
	//	read_variables(stream);
	//}
}

std::optional<std::reference_wrapper<variable>> find_variable(std::optional<int> scope_id, const std::string& name) {
	if (auto scope = find_scope(scope_id)) {
		return scope->find(name);
	} else {
		return std::nullopt;
	}
}

void create_variable(std::optional<int> scope_id, variable variable, bool replace) {
	if (auto scope = find_scope(scope_id)) {
		scope->add(variable, replace);
	}
}

void delete_variable(std::optional<int> scope_id, const std::string& name) {
	if (auto scope = find_scope(scope_id)) {
		scope->remove(name);
	}
}

void for_each_variable_in_scope(std::optional<int> scope_id, const std::function<void(const variable&)>& function) {
	if (auto scope = find_scope(scope_id)) {
		scope->for_each(function);
	}
}

}

std::ostream& operator<<(std::ostream& out, nfwk::variable_comparison type) {
	switch (type) {
	case nfwk::variable_comparison::equal: return out << "= Equals";
	case nfwk::variable_comparison::not_equal: return out << "!= Not Equals";
	case nfwk::variable_comparison::greater_than: return out << "> Greater Than";
	case nfwk::variable_comparison::less_than: return out << "< Less Than";
	case nfwk::variable_comparison::equal_or_greater_than: return out << ">= Equal or Greater Than";
	case nfwk::variable_comparison::equal_or_less_than: return out << "<= Equal or Less Than";
	default: return out << "Invalid comparison";
	}
}

std::ostream& operator<<(std::ostream& out, nfwk::variable_type type) {
	switch (type) {
	case nfwk::variable_type::string: return out << "String";
	case nfwk::variable_type::integer: return out << "Integer";
	case nfwk::variable_type::boolean: return out << "Boolean";
	case nfwk::variable_type::floating: return out << "Float";
	default: return out << "Invalid variable type";
	}
}

std::ostream& operator<<(std::ostream& out, nfwk::variable_operator type) {
	switch (type) {
	case nfwk::variable_operator::set: return out << "Set";
	case nfwk::variable_operator::negate: return out << "Negate";
	case nfwk::variable_operator::add: return out << "Add";
	case nfwk::variable_operator::multiply: return out << "Multiply";
	case nfwk::variable_operator::divide: return out << "Divide";
	default: return out << "Invalid operator";
	}
}
