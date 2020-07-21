#pragma once

#include "io.hpp"
#include "event.hpp"
#include "transform.hpp"
#include "variables.hpp"

#define NFWK_SCRIPT_NODE_EXPLICIT(BASE, TYPE, NAME, CATEGORY) \
	int type() const override { return (BASE) + (TYPE); }\
	std::string_view get_name() const override { return NAME; }\
	static constexpr int full_type{ (BASE) + (TYPE) };\
	static constexpr int relative_type{ TYPE };\
	static constexpr std::string_view name{ NAME };\
	static constexpr std::string_view category{ CATEGORY }

#define NFWK_SCRIPT_CORE_NODE(TYPE, NAME, CATEGORY) NFWK_SCRIPT_NODE_EXPLICIT(0x0000, TYPE, NAME, CATEGORY)
#define NFWK_SCRIPT_USER_NODE(TYPE, NAME, CATEGORY) NFWK_SCRIPT_NODE_EXPLICIT(0xffff, TYPE, NAME, CATEGORY)

namespace no {

namespace internal {
void initialize_scripts();
}

class script_tree;

enum class node_output_type { unknown, variable, single, boolean };
enum class node_other_variable_type { value, local, global };

class node_output {
public:

	node_output(int to_node_id, int slot_index);

	int slot() const;
	int to_node() const;
	void set_to_node(int to_node);

private:

	int to_node_id{ -1 };
	int slot_index{ 0 };

};

class script_node {
public:

	friend class script_tree;

	int id{ -1 };
	std::optional<int> scope_id;

	transform2 transform; // used in editor

	virtual int type() const = 0;
	virtual node_output_type output_type() const = 0;
	virtual std::string_view get_name() const = 0;

	virtual std::optional<int> process() {
		return std::nullopt;
	}

	virtual void write(io_stream& stream) const;
	virtual void read(io_stream& stream);
	virtual bool update_editor() = 0;

	void delete_output_node(int node_id);
	void delete_output_slot(int slot);
	std::optional<int> get_output_node(int slot) const;
	std::optional<node_output> get_first_output() const;
	void add_output(std::optional<int> slot, int to_node_id);

	const std::vector<node_output>& get_outputs() const;
	int used_output_slots_count() const;

protected:

	script_tree* tree{ nullptr };
	std::vector<node_output> outputs;

};

struct script_node_constructor {
	std::function<script_node*()> constructor;
	std::string_view name;
	std::string_view category;
	int type{ -1 };
};

void register_script_node(int id, std::string_view name, std::string_view category, const std::function<script_node*()>& constructor);
const std::vector<script_node_constructor>& get_registered_script_nodes();

template<typename T>
void register_script_node() {
	register_script_node(T::full_type, T::name, T::category, [] {
		return new T{};
	});
}

struct node_choice_info {
	
	std::string text;
	std::optional<int> node_id;

	node_choice_info(std::string_view text, std::optional<int> node_id)
		: text{ text }, node_id{ node_id } {
	}

};

class script_tree {
public:

	struct {
		event<std::vector<node_choice_info>> choice;
	} events;

	std::string id;
	std::string name;

	int id_counter{ 0 };
	int start_node_id{ 0 }; // todo: when deleting node, make sure start node is valid
	std::unordered_map<int, script_node*> nodes;

	variable_registry* context{ nullptr };

	void write(io_stream& stream) const;
	void read(io_stream& stream);

	void save() const;
	void load(const std::string& id);

	std::optional<int> current_node() const;

	void select_choice(int id);
	void process_entry_point();

private:

	bool process_choice_selection();
	void prepare_message();
	std::vector<int> process_current_and_get_choices();
	std::optional<int> process_nodes_get_choice(std::optional<int> node_id, int type);
	std::optional<int> process_non_interactive_node(int node_id, int type);

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

	NFWK_SCRIPT_CORE_NODE(0, "Message", "");

	std::string text{ "Example text" };

	node_output_type output_type() const override {
		return node_output_type::variable;
	}

	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

class choice_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(1, "Choice", "");

	std::string text{ "Example text" };

	node_output_type output_type() const override {
		return node_output_type::single;
	}

	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

class compare_variable_node : public condition_node {
public:

	NFWK_SCRIPT_CORE_NODE(2, "Compare variable", "Variables");

	bool is_global{ false };
	node_other_variable_type other_type{ node_other_variable_type::value };
	std::string variable_name;
	std::string comparison_value;
	variable_comparison comparison_operator{ variable_comparison::equal };

	std::optional<int> process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

class modify_variable_node : public effect_node {
public:

	NFWK_SCRIPT_CORE_NODE(3, "Modify variable", "Variables");

	bool is_global{ false };
	node_other_variable_type other_type{ node_other_variable_type::value };
	std::string variable_name;
	std::string modify_value;
	variable_modification modify_operator{ variable_modification::set };

	std::optional<int> process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

class create_variable_node : public effect_node {
public:

	NFWK_SCRIPT_CORE_NODE(4, "Create variable", "Variables");

	variable new_variable;
	bool is_global{ false };
	bool overwrite{ false };

	std::optional<int> process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

class variable_exists_node : public condition_node {
public:

	NFWK_SCRIPT_CORE_NODE(5, "If variable exists", "Variables");

	bool is_global{ false };
	std::string variable_name;

	std::optional<int> process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

class delete_variable_node : public effect_node {
public:

	NFWK_SCRIPT_CORE_NODE(6, "Delete variable", "Variables");

	bool is_global{ false };
	std::string variable_name;

	std::optional<int> process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

class random_output_node : public script_node {
public:

	NFWK_SCRIPT_CORE_NODE(7, "Random output", "Random");

	node_output_type output_type() const override {
		return node_output_type::variable;
	}

	std::optional<int> process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

class random_condition_node : public condition_node {
public:

	NFWK_SCRIPT_CORE_NODE(8, "Random true/false", "Random");

	int percent{ 50 };

	std::optional<int> process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

class execute_node : public effect_node {
public:

	NFWK_SCRIPT_CORE_NODE(9, "Execute script", "");

	std::string script;

	std::optional<int> process() override;
	void write(io_stream& stream) const override;
	void read(io_stream& stream) override;
	bool update_editor() override;

};

template<typename Code>
class code_node : public effect_node {
public:

	NFWK_SCRIPT_CORE_NODE(10, "Code", "");

	Code code;

	std::optional<int> process() override {
		code();
		return 0;
	}

	void write(io_stream& stream) const override {
		script_node::write(stream);
	}

	void read(io_stream& stream) override {
		script_node::read(stream);
	}

	bool update_editor() override {
		return false;
	}

};

using code_function_node = code_node<std::function<void()>>;

}
