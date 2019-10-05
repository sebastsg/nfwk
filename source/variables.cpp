#include "variables.hpp"
#include "debug.hpp"
#include "loop.hpp"

namespace no {

static bool compare_var_greater_than(variable_type type, const std::string& a, const std::string& b) {
	switch (type) {
	case variable_type::string: return a.size() > b.size();
	case variable_type::integer: return std::stoi(a) > std::stoi(b);
	case variable_type::floating: return std::stof(a) > std::stof(b);
	case variable_type::boolean: return a == "1";
	default: return false;
	}
}

static void modify_var_add(variable& var, const std::string& value) {
	switch (var.type) {
	case variable_type::string: var.value += value; return;
	case variable_type::integer: var.value = std::to_string(std::stoi(var.value) + std::stoi(value)); return;
	case variable_type::floating: var.value = std::to_string(std::stof(var.value) + std::stof(value)); return;
	case variable_type::boolean: var.value = std::to_string(std::stoi(var.value) + std::stoi(value)); return;
	default: return;
	}
}

static void modify_var_multiply(variable& var, const std::string& value) {
	switch (var.type) {
	case variable_type::string: WARNING("Multiplying a string does not make any sense."); return;
	case variable_type::integer: var.value = std::to_string(std::stoi(var.value) * std::stoi(value)); return;
	case variable_type::floating: var.value = std::to_string(std::stof(var.value) * std::stof(value)); return;
	case variable_type::boolean: var.value = std::to_string(std::stoi(var.value) * std::stoi(value)); return;
	default: return;
	}
}

static void modify_var_divide(variable& var, const std::string& value) {
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

bool variable::compare(const std::string& b, variable_comparison comp_operator) const {
	switch (comp_operator) {
	case variable_comparison::equal: return value == b;
	case variable_comparison::not_equal: return value != b;
	case variable_comparison::greater_than: return compare_var_greater_than(type, value, b);
	case variable_comparison::less_than: return compare_var_greater_than(type, b, value);
	case variable_comparison::equal_or_greater_than: return compare_var_greater_than(type, value, b) || value == b;
	case variable_comparison::equal_or_less_than: return compare_var_greater_than(type, b, value) || value == b;
	default: return false;
	}
}

void variable::modify(const std::string& new_value, variable_modification mod_operator) {
	switch (mod_operator) {
	case variable_modification::set: value = new_value; return;
	case variable_modification::negate: value = std::to_string(!std::stoi(value)); return;
	case variable_modification::add: modify_var_add(*this, new_value); return;
	case variable_modification::multiply: modify_var_multiply(*this, new_value); return;
	case variable_modification::divide: modify_var_divide(*this, new_value); return;
	default: return;
	}
}

static void write_game_variable(io_stream& stream, const variable& var) {
	stream.write((int32_t)var.type);
	stream.write(var.name);
	stream.write(var.value);
	stream.write<uint8_t>(var.is_persistent ? 1 : 0);
}

static variable read_game_variable(io_stream& stream) {
	variable var;
	var.type = static_cast<variable_type>(stream.read<int32_t>());
	var.name = stream.read<std::string>();
	var.value = stream.read<std::string>();
	var.is_persistent = (stream.read<uint8_t>() != 0);
	return var;
}

variable::variable(variable_type type, std::string name, std::string value, bool persistent)
	: type{ type }, name{ std::move(name) }, value{ std::move(value) }, is_persistent{ persistent } {
}

variable* variable_map::global(const std::string& name) {
	for (auto& i : globals) {
		if (i.name == name) {
			return &i;
		}
	}
	return nullptr;
}

variable* variable_map::local(int scope_id, const std::string& name) {
	if (auto scope{ locals.find(scope_id) }; scope != locals.end()) {
		for (auto& var : scope->second) {
			if (var.name == name) {
				return &var;
			}
		}
	}
	return nullptr;
}

void variable_map::create_global(variable var) {
	if (global(var.name)) {
		WARNING("Variable " << var.name << " already exists.");
		return;
	}
	globals.push_back(var);
}

void variable_map::create_local(int scope_id, variable var) {
	if (local(scope_id, var.name)) {
		WARNING("Variable " << var.name << " already exists.");
		return;
	}
	locals[scope_id].push_back(var);
}

void variable_map::delete_global(const std::string& name) {
	for (size_t i{ 0 }; i < globals.size(); i++) {
		if (globals[i].name == name) {
			globals.erase(globals.begin() + i);
			break;
		}
	}
}

void variable_map::delete_local(int scope_id, const std::string& name) {
	std::vector<variable>* scope_locals = &locals[scope_id];
	for (size_t i{ 0 }; i < scope_locals->size(); i++) {
		if (scope_locals->at(i).name == name) {
			scope_locals->erase(scope_locals->begin() + i);
			break;
		}
	}
}

void variable_map::for_each_global(const std::function<void(const variable&)>& function) const {
	for (const auto& var : globals) {
		function(var);
	}
}

void variable_map::for_each_local(const std::function<void(int, const variable&)>& function) const {
	for (const auto& scope : locals) {
		for (auto& var : scope.second) {
			function(scope.first, var);
		}
	}
}

void variable_map::write(io_stream& stream) const {
	stream.write<int32_t>(static_cast<int32_t>(globals.size()));
	for (const auto& i : globals) {
		write_game_variable(stream, i);
	}
	stream.write<int32_t>(static_cast<int32_t>(locals.size()));
	for (const auto& i : locals) {
		stream.write<int32_t>(i.first);
		stream.write<int32_t>(static_cast<int32_t>(i.second.size()));
		for (const auto& j : i.second) {
			write_game_variable(stream, j);
		}
	}
}

void variable_map::read(io_stream& stream) {
	const int global_count{ stream.read<int32_t>() };
	for (int i{ 0 }; i < global_count; i++) {
		globals.push_back(read_game_variable(stream));
	}
	const int scope_count{ stream.read<int32_t>() };
	for (int i{ 0 }; i < scope_count; i++) {
		const int scope_id{ stream.read<int32_t>() };
		const int var_count{ stream.read<int32_t>() };
		for (int j{ 0 }; j < var_count; j++) {
			locals[scope_id].push_back(read_game_variable(stream));
		}
	}
}

}
