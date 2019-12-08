#pragma once

#include "io.hpp"
#include "event.hpp"
#include "transform.hpp"
#include "variables.hpp"

namespace no {

class script_tree;

enum class node_output_type { unknown, variable, single, boolean };
enum class node_other_var_type { value, local, global };

struct node_output {
	int node_id{ -1 };
	int out_id{ 0 };
};

class script_node {
public:

	friend class script_tree;

	int id{ -1 };
	int scope_id{ -1 };
	std::vector<node_output> out;
	transform3 transform; // transform is only used in editor

	virtual int type() const = 0;
	virtual node_output_type output_type() const = 0;

	virtual int process() {
		return -1;
	}

	virtual void write(io_stream& stream);
	virtual void read(io_stream& stream);

	void remove_output_node(int node_id);
	void remove_output_type(int out_id);
	int get_output(int out_id);
	int get_first_output();
	void set_output_node(int out_id, int node_id);

protected:

	script_tree* tree{ nullptr };

};

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

	// context:
	variable_map* variables{ nullptr };

	void write(io_stream& stream) const;
	void read(io_stream& stream);

	void save() const;
	void load(int id);

	int current_node() const;

	void select_choice(int id);
	void process_entry_point();

private:

	bool process_choice_selection();
	void prepare_message();
	std::vector<int> process_current_and_get_choices();
	int process_nodes_get_choice(int id, int type);
	int process_non_ui_node(int id, int type);

	int current_node_id{ 0 };

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

	std::string text{ "Example text" };

	int type() const override {
		return 0;
	}

	node_output_type output_type() const override {
		return node_output_type::variable;
	}

	void write(io_stream& stream) override;
	void read(io_stream& stream) override;

};

class choice_node : public script_node {
public:

	std::string text{ "Example text" };

	int type() const override {
		return 1;
	}

	node_output_type output_type() const override {
		return node_output_type::single;
	}

	void write(io_stream& stream) override;
	void read(io_stream& stream) override;

};


class var_condition_node : public condition_node {
public:

	bool is_global{ false };
	node_other_var_type other_type{ node_other_var_type::value };
	std::string var_name;
	std::string comp_value;
	variable_comparison comp_operator{ variable_comparison::equal };

	int type() const override {
		return 2;
	}

	int process() override;
	void write(io_stream& stream) override;
	void read(io_stream& stream) override;

};

class modify_var_node : public effect_node {
public:

	bool is_global{ false };
	node_other_var_type other_type{ node_other_var_type::value };
	std::string var_name;
	std::string mod_value;
	variable_modification mod_operator{ variable_modification::set };

	int type() const override {
		return 3;
	}

	int process() override;
	void write(io_stream& stream) override;
	void read(io_stream& stream) override;

};

class create_var_node : public effect_node {
public:

	variable var;
	bool is_global{ false };
	bool overwrite{ false };

	int type() const override {
		return 4;
	}

	int process() override;
	void write(io_stream& stream) override;
	void read(io_stream& stream) override;

};

class var_exists_node : public condition_node {
public:

	bool is_global{ false };
	std::string var_name;

	int type() const override {
		return 5;
	}

	int process() override;
	void write(io_stream& stream) override;
	void read(io_stream& stream) override;

};

class delete_var_node : public effect_node {
public:

	bool is_global{ false };
	std::string var_name;

	int type() const override {
		return 6;
	}

	int process() override;
	void write(io_stream& stream) override;
	void read(io_stream& stream) override;

};

class random_node : public script_node {
public:

	int type() const override {
		return 7;
	}

	node_output_type output_type() const override {
		return node_output_type::variable;
	}

	int process() override;
	void write(io_stream& stream) override;
	void read(io_stream& stream) override;

};

class random_condition_node : public condition_node {
public:

	int percent{ 50 };

	int type() const override {
		return 8;
	}

	int process() override;
	void write(io_stream& stream) override;
	void read(io_stream& stream) override;

};

}
