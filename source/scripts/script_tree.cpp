#include "scripts/script_tree.hpp"
#include "scripts/script_node.hpp"
#include "io.hpp"
#include "log.hpp"
#include "assert.hpp"
#include "objects/objects.hpp"

namespace nfwk {

std::unordered_map<int, event<const script_node*>> script_tree::node_shown;

script_tree::script_tree(const script_node_factory& factory) : node_factory{ factory } {

}

void script_tree::write(io_stream& stream) const {
	stream.write_string(id);
	stream.write_string(name);
	stream.write_size(nodes.size());
	for (const auto& node : nodes) {
		stream.write_bool(node != nullptr);
		if (node) {
			stream.write<std::int32_t>(node->type());
			node->write(stream);
		}
	}
	stream.write_optional<std::int32_t>(start_node_id);
}

void script_tree::read(io_stream& stream) {
	if (stream.size_left_to_read() == 0) {
		return;
	}
	id = stream.read_string();
	name = stream.read_string();
	const auto node_count = stream.read_size();
	for (std::size_t i{ 0 }; i < node_count; i++) {
		if (stream.read_bool()) {
			const auto type = stream.read<std::int32_t>();
			auto node = node_factory.create_node(type);
			node->tree = this;
			node->read(stream);
			ASSERT(i == node->id);
			nodes.push_back(node);
		} else {
			nodes.push_back(nullptr);
		}
	}
	start_node_id = stream.read_optional<std::int32_t>();
	rebuild_valid_nodes();
	ASSERT(is_identifier_normalized(id));
}

std::optional<int> script_tree::current_node() const {
	return current_node_id;
}

std::shared_ptr<script_node> script_tree::get_node(int node_id) {
	return nodes[node_id];
}

void script_tree::set_start_node(std::optional<int> node_id) {
	start_node_id = node_id;
}

std::optional<int> script_tree::get_start_node_id() const {
	return start_node_id;
}

void script_tree::delete_node(int id) {
	if (start_node_id == id) {
		start_node_id = std::nullopt;
	}
	nodes[id] = nullptr;
	rebuild_valid_nodes();
}

void script_tree::add_node(std::shared_ptr<script_node> node) {
	for (int node_id{ 0 }; node_id < static_cast<int>(nodes.size()); node_id++) {
		if (!nodes[node_id]) {
			node->id = node_id;
			nodes[node_id] = node;
			valid_nodes.push_back(node);
			return;
		}
	}
	node->id = static_cast<int>(nodes.size());
	nodes.emplace_back(std::move(node));
	rebuild_valid_nodes();
}

const std::vector<std::shared_ptr<script_node>>& script_tree::get_nodes() const {
	return valid_nodes;
}

bool script_tree::process_entry_point() {
	ASSERT(start_node_id.has_value());
	event_fired = false;
	current_node_id = start_node_id;
	while (process_next_node());
	if (!event_fired) {
		on_done.emit();
	}
	return event_fired;
}

bool script_tree::process_output(int node_id, int slot) {
	event_fired = false;
	current_node_id = nodes[node_id]->get_output_node(slot);
	while (process_next_node());
	if (!event_fired) {
		on_done.emit();
	}
	return event_fired;
}

bool script_tree::process_outputs(int node_id) {
	event_fired = false;
	for (const auto& output : nodes[node_id]->get_outputs()) {
		current_node_id = output.to_node();
		while (process_next_node());
	}
	if (!event_fired) {
		on_done.emit();
	}
	return event_fired;
}

bool script_tree::process_next_node() {
	if (current_node_id.has_value()) {
		const auto& node = *nodes[current_node_id.value()];
		if (node.is_interactive()) {
			event_fired = true;
			on_interactive_node(node);
		}
		current_node_id = process_node(node);
	}
	return current_node_id.has_value();
}

std::optional<int> script_tree::process_node(const script_node& node) {
	if (const auto slot = node.process()) {
		return node.get_output_node(slot.value());
	} else {
		return std::nullopt;
	}
}

void script_tree::on_interactive_node(const script_node& node) {
	const auto& node_event = node_shown[node.type()];
	node_event.emit(&node);
}

#if 0
void script_tree::prepare_interactive_node() {
	ASSERT(current_node_id.has_value());
	std::vector<script_node_interaction> interactions;
	const auto& current_node = nodes[current_node_id.value()];
	for (const int node_id : process_outputs_and_get_interactive_nodes(current_node->outputs)) {
		if (const auto& node = nodes[node_id]) {
			interactions.emplace_back(node_id);
		} else {
			error(scripts::log, u8"{} is not a node!", node_id);
		}
	}
	ASSERT(!interactions.empty());
	on_interaction.emit(interactions);
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

std::optional<int> script_tree::process_nodes_and_get_interactive_node(std::shared_ptr<script_node> node) {
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
#endif

void script_tree::rebuild_valid_nodes() {
	valid_nodes.clear();
	for (const auto& node : nodes) {
		if (node) {
			valid_nodes.push_back(node);
		}
	}
}

}
