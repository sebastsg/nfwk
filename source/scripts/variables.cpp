#include "scripts/variables.hpp"
#include "debug.hpp"
#include "loop.hpp"

namespace no {

bool variable::compare(const std::string& right, variable_comparison comparison_operator) const {
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

bool variable::compare(int right, variable_comparison comparison_operator) const {
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

bool variable::compare(float right, variable_comparison comparison_operator) const {
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

bool variable::compare(const variable& right, variable_comparison comparison_operator) const {
	switch (right.type) {
	case variable_type::string: return compare(std::get<std::string>(right.value), comparison_operator);
	case variable_type::integer:
	case variable_type::boolean: return compare(std::get<int>(right.value), comparison_operator);
	case variable_type::floating: return compare(std::get<float>(right.value), comparison_operator);
	default: return false;
	}
}

void variable::modify(const std::string& right, variable_operator modify_operator) {
	switch (modify_operator) {
	case variable_operator::set:
		value = right;
		break;
	case variable_operator::add:
		std::get<std::string>(value) += right;
		break;
	default:
		WARNING_X("scripts", "Cannot run operator " << modify_operator << " on a string - " << name);
		break;
	}
}

void variable::modify(int right, variable_operator modify_operator) {
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
			WARNING_X("scripts", "Prevented division of " << name << " by 0. Setting the value to 0 to minimize errors.");
			std::get<int>(value) = 0;
		}
		break;
	}
}

void variable::modify(float right, variable_operator modify_operator) {
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
			WARNING_X("scripts", "Prevented division of " << name << " by 0.0. Setting the value to 0.0 to minimize errors.");
			std::get<float>(value) = 0.0f;
		}
		break;
	}
}

void variable::modify(const variable& right, variable_operator modify_operator) {
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

void variable::write(io_stream& stream) const {
	stream.write(static_cast<int32_t>(type));
	stream.write(name);
	switch (type) {
	case variable_type::string:
		stream.write(std::get<std::string>(value));
		break;
	case variable_type::integer:
		stream.write<int32_t>(std::get<int>(value));
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

void variable::read(io_stream& stream) {
	type = static_cast<variable_type>(stream.read<int32_t>());
	name = stream.read<std::string>();
	switch (type) {
	case variable_type::string:
		value = stream.read<std::string>();
		break;
	case variable_type::integer:
		value = static_cast<int>(stream.read<int32_t>());
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

variable::variable(io_stream& stream) {
	read(stream);
}

variable_scope::variable_scope(std::optional<int> id) : scope_id{ id } {

}

variable_scope::variable_scope(io_stream& stream)  {
	read(stream);
}

std::optional<int> variable_scope::id() const {
	return scope_id;
}

std::optional<std::reference_wrapper<variable>> variable_scope::find(const std::string& name) {
	for (auto& variable : variables) {
		if (variable.name == name) {
			return variable;
		}
	}
	return std::nullopt;
}

void variable_scope::add(variable variable) {
	if (!find(variable.name)) {
		variables.emplace_back(variable);
	} else {
		WARNING_X("scripts", "Variable " << variable.name << " already exists.");
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
	stream.write_optional<int32_t>(scope_id);
	stream.write(static_cast<int32_t>(variables.size()));
	for (const auto& variable : variables) {
		variable.write(stream);
	}
}

void variable_scope::read(io_stream& stream) {
	scope_id = stream.read_optional<int32_t>();
	const int32_t count{ stream.read<int32_t>() };
	for (int32_t i{ 0 }; i < count; i++) {
		variables.emplace_back(stream);
	}
}

std::optional<std::reference_wrapper<variable>> variable_registry::find(std::optional<int> scope_id, const std::string& name) {
	if (auto scope = find_scope(scope_id)) {
		return scope->find(name);
	} else {
		return std::nullopt;
	}
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
	for (const auto& scope : scopes) {
		scope.write(stream);
	}
}

void variable_registry::read(io_stream& stream) {
	const int32_t count{ stream.read<int32_t>() };
	for (int32_t i{ 0 }; i < count; i++) {
		scopes.emplace_back(stream);
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

std::ostream& operator<<(std::ostream& out, no::variable_comparison type) {
	switch (type) {
	case no::variable_comparison::equal: return out << "= Equals";
	case no::variable_comparison::not_equal: return out << "!= Not Equals";
	case no::variable_comparison::greater_than: return out << "> Greater Than";
	case no::variable_comparison::less_than: return out << "< Less Than";
	case no::variable_comparison::equal_or_greater_than: return out << ">= Equal or Greater Than";
	case no::variable_comparison::equal_or_less_than: return out << "<= Equal or Less Than";
	default: return out << "Invalid comparison";
	}
}

std::ostream& operator<<(std::ostream& out, no::variable_type type) {
	switch (type) {
	case no::variable_type::string: return out << "String";
	case no::variable_type::integer: return out << "Integer";
	case no::variable_type::boolean: return out << "Boolean";
	case no::variable_type::floating: return out << "Float";
	default: return out << "Invalid variable type";
	}
}

std::ostream& operator<<(std::ostream& out, no::variable_operator type) {
	switch (type) {
	case no::variable_operator::set: return out << "Set";
	case no::variable_operator::negate: return out << "Negate";
	case no::variable_operator::add: return out << "Add";
	case no::variable_operator::multiply: return out << "Multiply";
	case no::variable_operator::divide: return out << "Divide";
	default: return out << "Invalid operator";
	}
}