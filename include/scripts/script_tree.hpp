#pragma once

#include "event.hpp"

#include <optional>

#include "objects/objects.hpp"

namespace nfwk {

class variable_registry;
class script_node;
class script_node_output;
class io_stream;
class script_node_factory;

class script_node_interaction {
public:

	script_node_interaction(std::optional<int> node_id) : node_id{ node_id } {}

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

class script_context {
public:

	std::shared_ptr<variable_registry> variables;
	std::shared_ptr<script_event_container> events;
	std::shared_ptr<object_manager> objects;
	
};

class script_tree {

	event<std::vector<script_node_interaction>> on_interaction;
public:

	static constexpr std::u8string_view file_extension{ u8".nfwk-script" };

	event<> on_done;
	
	std::u8string id;
	std::u8string name;

	script_context context;

	script_tree(const script_node_factory& factory);

	void write(io_stream& stream) const;
	void read(io_stream& stream);

	std::optional<int> current_node() const;
	std::shared_ptr<script_node> get_node(int node_id);
	void set_start_node(std::optional<int> node_id);
	std::optional<int> get_start_node_id() const;
	void delete_node(int node_id);
	void add_node(std::shared_ptr<script_node> node);
	const std::vector<std::shared_ptr<script_node>>& get_nodes() const;

	bool process_entry_point();
	bool process_output(int node_id, int slot);
	bool process_outputs(int node_id);

	template<typename Node>
	[[nodiscard]] static event_listener listen_to_interactive_node(const std::function<void(const Node&)>& handler) {
		return node_shown[Node::full_type].listen([handler](const script_node& base_node) {
			if (const auto& node = static_cast<const Node&>(base_node)) {
				handler(node);
			}
		});
	}

private:

	static void on_interactive_node(const script_node& node);
	static std::optional<int> process_node(const script_node& node);

	static std::unordered_map<int, event<const script_node*>> node_shown;

	bool process_next_node();
	void rebuild_valid_nodes();
	
#if 0
	void prepare_interactive_node();
	std::vector<int> process_outputs_and_get_interactive_nodes(const std::vector<script_node_output>& outputs);
	std::optional<int> process_nodes_and_get_interactive_node(std::shared_ptr<script_node> node);
#endif

	std::optional<int> current_node_id;
	std::vector<std::shared_ptr<script_node>> nodes;
	std::vector<std::shared_ptr<script_node>> valid_nodes;
	std::optional<int> start_node_id;
	const script_node_factory& node_factory;
	bool event_fired{ false };

};

}
