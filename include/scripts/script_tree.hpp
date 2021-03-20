#pragma once

#include "event.hpp"

#include <optional>

namespace nfwk {

class variable_registry;
class script_node;
class script_node_output;
class io_stream;

class script_node_interaction {
public:

	script_node_interaction(std::optional<int> node_id)
		: node_id{ node_id } {
	}

	std::optional<int> get_node_id() const {
		return node_id;
	}

	void select_output(int selected_slot) {
		slot = selected_slot;
	}

private:

	std::optional<int> node_id;
	std::optional<int> slot;


};

class script_tree {
public:

	struct {
		event<std::vector<script_node_interaction>> interactions;
	} events;

	std::string id;
	std::string name;

	variable_registry* context{ nullptr };

	void write(io_stream& stream) const;
	void read(io_stream& stream);

	std::optional<int> current_node() const;
	script_node* get_node(int id);
	void set_start_node(std::optional<int> id);
	std::optional<int> get_start_node_id() const;
	void delete_node(int id);
	void add_node(script_node* node);
	const std::vector<script_node*>& get_nodes() const;

	void process_entry_point();
	void process_interactive_output(int id, int slot);

private:

	bool process_next_node();
	void prepare_interactive_node();
	std::vector<int> process_outputs_and_get_interactive_nodes(const std::vector<script_node_output>& outputs);
	std::optional<int> process_nodes_and_get_interactive_node(script_node* node);
	std::optional<int> process_node(script_node& node);

	void rebuild_valid_nodes();

	std::optional<int> current_node_id;
	std::vector<script_node*> nodes;
	std::vector<script_node*> valid_nodes;
	std::optional<int> start_node_id;

};

}
