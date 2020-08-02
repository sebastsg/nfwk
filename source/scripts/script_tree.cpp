#include "scripts/script_tree.hpp"
#include "scripts/script_node.hpp"
#include "io.hpp"
#include "debug.hpp"
#include "assets.hpp"

namespace no {

void script_tree::write(io_stream& stream) const {
	stream.write(id);
	stream.write(name);
	stream.write<int32_t>(static_cast<int32_t>(nodes.size()));
	for (const auto* node : nodes) {
		stream.write<bool>(node != nullptr);
		if (node) {
			stream.write<int32_t>(node->type());
			node->write(stream);
		}
	}
	stream.write_optional<int32_t>(start_node_id);
}

void script_tree::read(io_stream& stream) {
	id = stream.read<std::string>();
	name = stream.read<std::string>();
	const auto node_count = stream.read<int32_t>();
	for (int32_t i{ 0 }; i < node_count; i++) {
		if (stream.read<bool>()) {
			const auto type = stream.read<int32_t>();
			auto node = create_script_node(type);
			node->tree = this;
			node->read(stream);
			ASSERT(i == node->id);
			nodes.push_back(node);
		} else {
			nodes.push_back(nullptr);
		}
	}
	start_node_id = stream.read_optional<int32_t>();
	rebuild_valid_nodes();
}

void script_tree::save() const {
	io_stream stream;
	write(stream);
	file::write(asset_path("scripts/" + id + ".ns"), stream);
}

void script_tree::load(const std::string& id) {
	io_stream stream;
	file::read(asset_path("scripts/" + id + ".ns"), stream);
	if (stream.size_left_to_read() > 0) {
		read(stream);
	}
}

std::optional<int> script_tree::current_node() const {
	return current_node_id;
}

script_node* script_tree::get_node(int id) {
	return nodes[id];
}

void script_tree::set_start_node(std::optional<int> id) {
	start_node_id = id;
}

std::optional<int> script_tree::get_start_node_id() const {
	return start_node_id;
}

void script_tree::delete_node(int id) {
	if (start_node_id == id) {
		start_node_id = std::nullopt;
	}
	delete nodes[id];
	nodes[id] = nullptr;
	rebuild_valid_nodes();
}

void script_tree::add_node(script_node* node) {
	node->id = static_cast<int>(nodes.size());
	nodes.push_back(node);
	rebuild_valid_nodes();
}

const std::vector<script_node*>& script_tree::get_nodes() const {
	return valid_nodes;
}

void script_tree::process_entry_point() {
	current_node_id = start_node_id;
	while (process_next_node());
}

void script_tree::process_interactive_output(int node_id, int slot) {
	if (const auto* node = nodes[node_id]) {
		current_node_id = node->get_output_node(slot);
		while (process_next_node());
	} else {
		WARNING_X("scripts", "Node not found: " << node_id << ". Script: " << id);
	}
}

bool script_tree::process_next_node() {
	if (!current_node_id.has_value()) {
		return false;
	}
	auto node = nodes[current_node_id.value()];
	if (node->has_interactive_output_nodes()) {
		prepare_interactive_node();
		return false;
	}
	if (!node->can_be_entry_point()) {
		WARNING_X("scripts", "This node cannot be the entry point of a script.");
		return false;
	}
	current_node_id = process_node(*node);
	return current_node_id.has_value();
}

void script_tree::prepare_interactive_node() {
	ASSERT(current_node_id.has_value());
	std::vector<script_node_interaction> interactions;
	const auto* current_node = nodes[current_node_id.value()];
	for (const int node_id : process_outputs_and_get_interactive_nodes(current_node->outputs)) {
		if (const auto* node = nodes[node_id]) {
			interactions.emplace_back(node_id);
		} else {
			BUG(choice << " is not a node!");
		}
	}
	ASSERT(!interactions.empty());
	events.interactions.emit(interactions);
}

std::vector<int> script_tree::process_outputs_and_get_interactive_nodes(const std::vector<script_node_output>& outputs) {
	std::vector<int> interactive_node_ids;
	for (const auto& output : outputs) {
		auto output_node = nodes[output.to_node()];
		ASSERT(output_node->is_interactive());
		if (output_node->is_interactive()) {
			interactive_node_ids.push_back(output.to_node());
		} else {
			if (const auto traversed = process_nodes_and_get_interactive_node(output_node)) {
				interactive_node_ids.push_back(traversed.value());
			}
		}
	}
	return interactive_node_ids;
}

std::optional<int> script_tree::process_nodes_and_get_interactive_node(script_node* node) {
	while (node && !node->has_interactive_output_nodes()) {
		if (node->is_interactive()) {
			return node->id;
		}
		if (const auto node_id = process_node(*node)) {
			node = nodes[node_id.value()];
			if (node->is_interactive()) {
				return node_id;
			}
		}
	}
	return std::nullopt;
}

std::optional<int> script_tree::process_node(script_node& node) {
	if (const auto slot = node.process()) {
		return node.get_output_node(slot.value());
	} else {
		return std::nullopt;
	}
}

void script_tree::rebuild_valid_nodes() {
	valid_nodes.clear();
	for (auto node : nodes) {
		if (node) {
			valid_nodes.push_back(node);
		}
	}
}

}
