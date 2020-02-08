#include "variables.hpp"
#include "debug.hpp"
#include "loop.hpp"

namespace no {

static bool compare_variable_greater_than(variable_type type, const std::string& a, const std::string& b) {
	switch (type) {
	case variable_type::string: return a.size() > b.size();
	case variable_type::integer: return std::stoi(a) > std::stoi(b);
	case variable_type::floating: return std::stof(a) > std::stof(b);
	case variable_type::boolean: return a == "1";
	default: return false;
	}
}

static void modify_variable_add(variable& var, const std::string& value) {
	switch (var.type) {
	case variable_type::string: var.value += value; return;
	case variable_type::integer: var.value = std::to_string(std::stoi(var.value) + std::stoi(value)); return;
	case variable_type::floating: var.value = std::to_string(std::stof(var.value) + std::stof(value)); return;
	case variable_type::boolean: var.value = std::to_string(std::stoi(var.value) + std::stoi(value)); return;
	default: return;
	}
}

static void modify_variable_multiply(variable& var, const std::string& value) {
	switch (var.type) {
	case variable_type::string: WARNING("Multiplying a string does not make any sense."); return;
	case variable_type::integer: var.value = std::to_string(std::stoi(var.value) * std::stoi(value)); return;
	case variable_type::floating: var.value = std::to_string(std::stof(var.value) * std::stof(value)); return;
	case variable_type::boolean: var.value = std::to_string(std::stoi(var.value) * std::stoi(value)); return;
	default: return;
	}
}

static void modify_variable_divide(variable& var, const std::string& value) {
	const int value_int{ std::stoi(value) };
	const float value_float{ std::stof(value) };
	if (value_int == 0 || value_float == 0.0f) {
		WARNING("Cannot divide " << var.name << " by " << value << " (division by zero)\nSetting to 0.");
		var.value = "0"; // most likely the desired output
		return;
	}
	switch (var.type) {
	case variable_type::string: WARNING("Dividing a string does not make any sense."); return;
	case variable_type::integer: var.value = std::to_string(std::stoi(var.value) / value_int); return;
	case variable_type::floating: var.value = std::to_string(std::stof(var.value) / value_float); return;
	case variable_type::boolean: var.value = std::to_string(std::stoi(var.value) / value_int); return;
	default: return;
	}
}

static void modify_variable_negate(variable& var) {
	if (var.type == variable_type::integer) {
		var.value = std::to_string(!std::stoi(var.value));
	} else if (var.type == variable_type::floating) {
		var.value = std::to_string(!std::stof(var.value));
	} else {
		WARNING("Cannot negate " << var.name << ". Not a number type: " << static_cast<int>(var.type));
	}
}

bool variable::compare(const std::string& b, variable_comparison comp_operator) const {
	switch (comp_operator) {
	case variable_comparison::equal: return value == b;
	case variable_comparison::not_equal: return value != b;
	case variable_comparison::greater_than: return compare_variable_greater_than(type, value, b);
	case variable_comparison::less_than: return compare_variable_greater_than(type, b, value);
	case variable_comparison::equal_or_greater_than: return compare_variable_greater_than(type, value, b) || value == b;
	case variable_comparison::equal_or_less_than: return compare_variable_greater_than(type, b, value) || value == b;
	default: return false;
	}
}

void variable::modify(const std::string& new_value, variable_modification modify_operator) {
	switch (modify_operator) {
	case variable_modification::set: value = new_value; return;
	case variable_modification::negate: modify_variable_negate(*this); return;
	case variable_modification::add: modify_variable_add(*this, new_value); return;
	case variable_modification::multiply: modify_variable_multiply(*this, new_value); return;
	case variable_modification::divide: modify_variable_divide(*this, new_value); return;
	default: return;
	}
}

static void write_game_variable(io_stream& stream, const variable& var) {
	stream.write(static_cast<int32_t>(var.type));
	stream.write(var.name);
	stream.write(var.value);
	stream.write<uint8_t>(var.persistent ? 1 : 0);
}

static variable read_game_variable(io_stream& stream) {
	variable var;
	var.type = static_cast<variable_type>(stream.read<int32_t>());
	var.name = stream.read<std::string>();
	var.value = stream.read<std::string>();
	var.persistent = (stream.read<uint8_t>() != 0);
	return var;
}

variable::variable(variable_type type, std::string name, std::string value, bool persistent)
	: type{ type }, name{ std::move(name) }, value{ std::move(value) }, persistent{ persistent } {
}

int variable_scope::id() const {
	return scope_id;
}

variable* variable_scope::find(const std::string& name) {
	for (auto& i : variables) {
		if (i.name == name) {
			return &i;
		}
	}
	return nullptr;
}

void variable_scope::add(variable variable) {
	if (!find(variable.name)) {
		variables.push_back(variable);
	} else {
		WARNING("Variable " << variable.name << " already exists.");
	}
}

void variable_scope::remove(const std::string& name) {
	for (size_t i{ 0 }; i < variables.size(); i++) {
		if (variables[i].name == name) {
			variables.erase(variables.begin() + i);
			break;
		}
	}
}

void variable_scope::for_each(const std::function<void(const variable&)>& function) const {
	for (const auto& variable : variables) {
		function(variable);
	}
}

void variable_scope::write(io_stream& stream) const {
	stream.write<int32_t>(scope_id);
	stream.write(static_cast<int32_t>(variables.size()));
	for (const auto& variable : variables) {
		write_game_variable(stream, variable);
	}
}

void variable_scope::read(io_stream& stream) {
	scope_id = stream.read<int32_t>();
	const int32_t count{ stream.read<int32_t>() };
	for (int32_t i{ 0 }; i < count; i++) {
		variables.push_back(read_game_variable(stream));
	}
}

variable* variable_registry::find(std::optional<int> scope_id, const std::string& name) {
	auto scope = find_scope(scope_id);
	return scope ? scope->find(name) : nullptr;
}

void variable_registry::add(std::optional<int> scope_id, variable variable) {
	if (auto scope = find_scope(scope_id)) {
		scope->add(variable);
	}
}

void variable_registry::remove(std::optional<int> scope_id, const std::string& name) {
	if (auto scope = find_scope(scope_id)) {
		scope->remove(name);
	}
}

void variable_registry::for_each(std::optional<int> scope_id, const std::function<void(const variable&)>& function) const {
	if (auto scope = find_scope(scope_id)) {
		scope->for_each(function);
	}
}

void variable_registry::write(io_stream& stream) const {
	stream.write(static_cast<int32_t>(scopes.size()));
	for (auto& scope : scopes) {
		scope.write(stream);
	}
}

void variable_registry::read(io_stream& stream) {
	const int32_t count{ stream.read<int32_t>() };
	for (int32_t i{ 0 }; i < count; i++) {
		scopes.emplace_back().read(stream);
	}
}

variable_scope* variable_registry::find_scope(std::optional<int> scope_id) {
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

const variable_scope* variable_registry::find_scope(std::optional<int> scope_id) const {
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

}
