#pragma once

#include "io.hpp"
#include "event.hpp"
#include "transform.hpp"
#include "variables.hpp"

#define NFWK_SCRIPT_NODE_EXPLICIT(BASE, TYPE) \
	int type() const override { return (BASE) + (TYPE); }\
	static constexpr int full_type{ (BASE) + (TYPE) };\
	static constexpr int relative_type { TYPE }

#define NFWK_SCRIPT_CORE_NODE(TYPE) NFWK_SCRIPT_NODE_EXPLICIT(0x0000, TYPE)
#define NFWK_SCRIPT_USER_NODE(TYPE) NFWK_SCRIPT_NODE_EXPLICIT(0xffff, TYPE)

namespace no {

class script_tree;

enum class node_output_type { unknown, variable, single, boolean };
enum class node_other_variable_type { value, local, global };

struct node_output {
	int node_id{ -1 };
	int out_id{ 0 };
	node_output(int node_id, int out_id) : node_id{ node_id }, out_id{ out_id } {}
};

class script_node {
public:

	friend class script_tree;

	int id{ -1 };
	int scope_id{ -1 };

	std::vector<node_output> out;
	transform2 transform; // transform is only used in editor

	virtual int type() const = 0;
	virtual node_output_type output_type() const = 0;

	virtual int process() {
		return -1;
	}

	virtual void write(io_stream& stream) const;
	virtual void read(io_stream& stream);

	void remove_output_node(int node_id);
	void remove_output_type(int out_id);
	int get_output(int out_id);
	int get_first_output();
	void set_output_node(int out_id, int node_id);

protected:

	script_tree* tree{ nullptr };

};

void register_script_node(int id, const std::function<script_node*()>& constructor);
void initialize_scripts();

template<typename T>
void register_script_node() {
	register_script_node(T::full_type, [] {
		return new T{};
	});
}

struct node_choice_info {
	std::string text;
	int node_id{ -1 };
};

class script_tree {
public:

	struct {
		event<std::vector<node_choice_info>> choice;
	} events;

	int id{ -1 };
	int id_counter{ 0 };
	int start_node_id{ 0 }; // todo: when deleting node, make sure start node is valid
	std::unordered_map<int, script_node*> nodes;

	variable_registry* context{ nullptr };

	void write(io_stream& stream) const;
	void read(io_stream& stream);

	void save() const;
	void load(int id);

	std::optional<int> current_node() const;

	void select_choice(int id);
	void process_entry_point();

private:

	bool process_choice_selection();
	void prepare_message();
	std::vector<int> process_current_and_get_choices();
	std::optional<int> process_nodes_get_choice(std::optional<int> id, int type);
	std::optional<int> process_non_interactive_node(int id, int type);

	std::optional<int> current_node_id;

};

class condition_node : public script_node {
public:

	node_output_type output_type() const override {
		return node_output_type::boolean;
	}

};

class effect_node : public script_node {
public:

	node_output_type output_type() const override {
		return node_output_type::single;
	}

};

class message_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(0);

	std::string text{ "Example text" };

	node_output_type output_type() const override {
		return node_output_type::variable;
	}

	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class choice_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(1);

	std::string text{ "Example text" };

	node_output_type output_type() const override {
		return node_output_type::single;
	}

	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class variable_condition_node : public condition_node {
public:

	NFWK_SCRIPT_CORE_NODE(2);

	bool is_global{ false };
	node_other_variable_type other_type{ node_other_variable_type::value };
	std::string variable_name;
	std::string comparison_value;
	variable_comparison comparison_operator{ variable_comparison::equal };

	int process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class modify_variable_node : public effect_node {
public:

	NFWK_SCRIPT_CORE_NODE(3);

	bool is_global{ false };
	node_other_variable_type other_type{ node_other_variable_type::value };
	std::string variable_name;
	std::string modify_value;
	variable_modification modify_operator{ variable_modification::set };

	int process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class create_variable_node : public effect_node {
public:

	NFWK_SCRIPT_CORE_NODE(4);

	variable new_variable;
	bool is_global{ false };
	bool overwrite{ false };

	int process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class variable_exists_node : public condition_node {
public:

	NFWK_SCRIPT_CORE_NODE(5);

	bool is_global{ false };
	std::string variable_name;

	int process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class delete_variable_node : public effect_node {
public:

	NFWK_SCRIPT_CORE_NODE(6);

	bool is_global{ false };
	std::string variable_name;

	int process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class random_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(7);

	node_output_type output_type() const override {
		return node_output_type::variable;
	}

	int process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class random_condition_node : public condition_node {
public:

	NFWK_SCRIPT_CORE_NODE(8);

	int percent{ 50 };

	int process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

class execute_node : public effect_node {
public:

	NFWK_SCRIPT_CORE_NODE(9);

	std::string script;

	int process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;

};

}
