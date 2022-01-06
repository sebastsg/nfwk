#pragma once

#include "scripts/script_node.hpp"
#include "scripts/variables.hpp"
#include "script_node_macro.hpp"

namespace nfwk::script {

class compare_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(2, "Compare variable", "Variables");

	bool is_global{ false };
	other_variable_type other_type{ other_variable_type::value };
	std::string variable_name;
	std::string comparison_value;
	variable_comparison comparison_operator{ variable_comparison::equal };

	output_type get_output_type() const override {
		return output_type::boolean;
	}

	std::optional<int> process(script_context& context) const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class modify_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(3, "Modify variable", "Variables");

	bool is_global{ false };
	other_variable_type other_type{ other_variable_type::value };
	std::string variable_name;
	std::string modify_value;
	variable_operator modify_operator{ variable_operator::set };

	output_type get_output_type() const override {
		return output_type::single;
	}

	std::optional<int> process(script_context& context) const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class create_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(4, "Create variable", "Variables");

	variable new_variable;
	bool is_global{ false };
	bool overwrite{ false };

	output_type get_output_type() const override {
		return output_type::single;
	}

	std::optional<int> process(script_context& context) const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class variable_exists_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(5, "If variable exists", "Variables");

	bool is_global{ false };
	std::string variable_name;

	output_type get_output_type() const override {
		return output_type::boolean;
	}

	std::optional<int> process(script_context& context) const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class delete_variable_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(6, "Delete variable", "Variables");

	bool is_global{ false };
	std::string variable_name;

	output_type get_output_type() const override {
		return output_type::single;
	}

	std::optional<int> process(script_context& context) const override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

}
