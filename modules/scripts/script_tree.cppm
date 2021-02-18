module;

#include "assert.hpp"

export module nfwk.scripts:tree;

import std.core;
import std.memory;
import std.threading;
import std.filesystem;
import nfwk.core;
import nfwk.assets;
import :node;
import :script_utility_functions;

export namespace nfwk {

class script_tree {
public:

	static constexpr std::string_view file_extension{ ".nfwk-script" };

	event<> on_done;

	std::string id;
	std::string name;

	script_tree() = default;
	script_tree(const script_tree&) = delete;
	script_tree(script_tree&&) = delete;

	~script_tree() {
		for (auto node : nodes) {
			delete node;
		}
	}

	script_tree& operator=(const script_tree&) = delete;
	script_tree& operator=(script_tree&&) = default;

	void write(io_stream& stream) const {
		ASSERT(is_identifier_normalized(id));
		stream.write(id);
		stream.write(name);
		stream.write(static_cast<std::int32_t>(nodes.size()));
		for (const auto* node : nodes) {
			stream.write<bool>(node != nullptr);
			if (node) {
				stream.write<std::int32_t>(node->type());
				node->write(stream);
			}
		}
		stream.write_optional<std::int32_t>(start_node_id);
	}

	void read(io_stream& stream) {
		id = stream.read<std::string>();
		name = stream.read<std::string>();
		const auto node_count = stream.read<std::int32_t>();
		for (std::int32_t i{ 0 }; i < node_count; i++) {
			if (stream.read<bool>()) {
				const auto type = stream.read<std::int32_t>();
				auto node = create_script_node(type);
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

	void save() const {
		io_stream stream;
		write(stream);
		write_file(get_path(), stream);
	}

	void load(std::string_view new_id) {
		ASSERT(is_identifier_normalized(new_id));
		id = new_id;
		if (io_stream stream{ get_path() }; !stream.empty()) {
			read(stream);
		}
	}

	std::optional<int> current_node() const {
		return current_node_id;
	}

	script_node* get_node(int id) {
		return nodes[id];
	}

	void set_start_node(std::optional<int> id) {
		start_node_id = id;
	}

	std::optional<int> get_start_node_id() const {
		return start_node_id;
	}

	void delete_node(int id) {
		if (start_node_id == id) {
			start_node_id = std::nullopt;
		}
		delete nodes[id];
		nodes[id] = nullptr;
		rebuild_valid_nodes();
	}

	void add_node(script_node* node) {
		for (int node_id{ 0 }; node_id < static_cast<int>(nodes.size()); node_id++) {
			if (!nodes[node_id]) {
				node->id = node_id;
				nodes[node_id] = node;
				valid_nodes.push_back(node);
				return;
			}
		}
		node->id = static_cast<int>(nodes.size());
		nodes.push_back(node);
		rebuild_valid_nodes();
	}

	const std::vector<script_node*>& get_nodes() const {
		return valid_nodes;
	}

	[[nodiscard]] bool process_entry_point() {
		ASSERT(start_node_id.has_value());
		event_fired = false;
		current_node_id = start_node_id;
		while (process_next_node());
		if (!event_fired) {
			on_done.emit();
		}
		return event_fired;
	}

	[[nodiscard]] bool process_output(int node_id, int slot) {
		event_fired = false;
		current_node_id = nodes[node_id]->get_output_node(slot);
		while (process_next_node());
		if (!event_fired) {
			on_done.emit();
		}
		return event_fired;
	}

	[[nodiscard]] bool process_outputs(int node_id) {
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

	template<typename Node>
	[[nodiscard]] static event_listener listen_to_interactive_node(const std::function<void(const Node&)>& handler) {
		return node_shown[Node::full_type].listen([handler](const script_node* base_node) {
			if (const auto* node = static_cast<const Node*>(base_node)) {
				handler(*node);
			}
		});
	}

private:

	static void on_interactive_node(const script_node& node) {
		auto& node_event = node_shown[node.type()];
		node_event.emit(&node);
	}

	[[nodiscard]] bool process_next_node() {
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

	[[nodiscard]] std::optional<int> process_node(const script_node& node) {
		if (const auto slot = node.process()) {
			return node.get_output_node(slot.value());
		} else {
			return std::nullopt;
		}
	}


	void rebuild_valid_nodes() {
		valid_nodes.clear();
		for (auto node : nodes) {
			if (node) {
				valid_nodes.push_back(node);
			}
		}
	}

	std::filesystem::path get_path() const {
		return asset_path("scripts/" + id + std::string{ file_extension });
	}

	static std::unordered_map<int, event<const script_node*>> node_shown;
	std::optional<int> current_node_id;
	std::vector<script_node*> nodes;
	std::vector<script_node*> valid_nodes;
	std::optional<int> start_node_id;
	bool event_fired{ false };

};

std::unordered_map<int, event<const script_node*>> script_tree::node_shown;

}

namespace nfwk {

#if 0
std::vector<std::unique_ptr<script_tree>> finished_scripts;
std::vector<std::pair<std::unique_ptr<script_tree>, event_listener>> running_scripts;
#endif

}

export namespace nfwk {

[[nodiscard]] script_tree* run_script(std::string_view id) {
#if 0
	ASSERT(is_identifier_normalized(id));
	finished_scripts.clear();
	auto& [script, done] = running_scripts.emplace_back(std::make_unique<script_tree>(), event_listener{});
	script->load(id);
	done = script->on_done.listen([id] {
		auto it = std::find_if(running_scripts.begin(), running_scripts.end(), [id](const auto& script) {
			return script.first->id == id;
		});
		if (it != running_scripts.end()) {
			finished_scripts.emplace_back(std::move(it->first));
			running_scripts.erase(it);
		}
	});
	if (script->process_entry_point()) {
		return script.get();
	} else {
		return nullptr;
	}
#endif
	return nullptr;
}

}
