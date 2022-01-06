#include "scripts/script_tree.hpp"
#include "scripts/script_node.hpp"
#include "io.hpp"
#include "log.hpp"
#include "assert.hpp"
#include "objects/objects.hpp"

namespace nfwk::script {

script_definition::script_definition(std::string id, std::string name, std::optional<int> start_node_id, std::vector<std::shared_ptr<script_node>> nodes)
	: id{ std::move(id) }, name{ std::move(name) }, start_node_id{ start_node_id }, nodes{ std::move(nodes) } {
	rebuild_valid_nodes();
}

void script_definition::write(io_stream& stream) const {
	stream.write_string(id);
	stream.write_string(name);
	stream.write_size(nodes.size());
	for (const auto& node : nodes) {
		stream.write_bool(node != nullptr);
		if (node) {
			stream.write_int32(node->type());
			node->write(stream);
		}
	}
	stream.write_optional<std::int32_t>(start_node_id);
}

std::shared_ptr<script_node> script_definition::get_node(int node_id) {
	return nodes[node_id];
}

void script_definition::set_start_node(std::optional<int> node_id) {
	start_node_id = node_id;
}

std::optional<int> script_definition::get_start_node_id() const {
	return start_node_id;
}

void script_definition::delete_node(int node_id) {
	if (start_node_id == node_id) {
		start_node_id = std::nullopt;
	}
	nodes[node_id] = nullptr;
	rebuild_valid_nodes();
}

void script_definition::add_node(std::shared_ptr<script_node> node) {
	for (int node_id{ 0 }; node_id < static_cast<int>(nodes.size()); node_id++) {
		if (!nodes[node_id]) {
			node->set_id(node_id);
			nodes[node_id] = node;
			valid_nodes.push_back(node);
			return;
		}
	}
	node->set_id(static_cast<int>(nodes.size()));
	nodes.emplace_back(std::move(node));
	rebuild_valid_nodes();
}

const std::vector<std::shared_ptr<script_node>>& script_definition::get_nodes() const {
	return valid_nodes;
}

void script_definition::rebuild_valid_nodes() {
	valid_nodes.clear();
	for (const auto& node : nodes) {
		if (node) {
			valid_nodes.push_back(node);
		}
	}
}

running_script::running_script(std::shared_ptr<script_definition> script, script_runner& runner)
	: script{ std::move(script) }, runner{ runner } {}

std::optional<int> running_script::get_current_node_id() const {
	return current_node_id;
}

bool running_script::is_done() const {
	return done;
}

bool running_script::process_entry_point() {
	ASSERT(script->get_start_node_id().has_value());
	event_fired = false;
	current_node_id = script->get_start_node_id();
	while (process_next_node());
	if (!event_fired) {
		done = true;
	}
	return event_fired;
}

bool running_script::process_output(int node_id, int slot) {
	event_fired = false;
	current_node_id = script->get_node(node_id)->get_output_node(slot);
	while (process_next_node());
	if (!event_fired) {
		done = true;
	}
	return event_fired;
}

bool running_script::process_outputs(int node_id) {
	event_fired = false;
	for (const auto& output : script->get_node(node_id)->get_outputs()) {
		current_node_id = output.to_node();
		while (process_next_node());
	}
	if (!event_fired) {
		done = true;
	}
	return event_fired;
}

bool running_script::process_next_node() {
	if (current_node_id.has_value()) {
		const auto& node = *script->get_node(current_node_id.value());
		if (node.is_interactive()) {
			event_fired = true;
			runner.on_interactive_node(node);
		}
		if (const auto slot = node.process(context)) {
			current_node_id = node.get_output_node(slot.value());
		} else {
			current_node_id = std::nullopt;
		}
	}
	return current_node_id.has_value();
}

script_loader::script_loader(const script_node_factory& factory) : node_factory{ factory } {

}

std::shared_ptr<script_definition> script_loader::load(const std::filesystem::path& path) const {
	std::vector<std::shared_ptr<script_node>> nodes;
	io_stream stream;
	read_file(path, stream);
	if (stream.size_left_to_read() == 0) {
		return nullptr;
	}
	const auto id = stream.read_string();
	ASSERT(is_identifier_normalized(id));

	const auto name = stream.read_string();
	const auto node_count = stream.read_size();
	for (std::size_t i{ 0 }; i < node_count; i++) {
		if (stream.read_bool()) {
			const auto type = stream.read_int32();
			auto node = node_factory.create_node(type);
			node->read(stream);
			ASSERT(i == node->get_id());
			nodes.push_back(node);
		} else {
			nodes.push_back(nullptr);
		}
	}
	const auto start_node_id = stream.read_optional_int32();
	return std::make_shared<script_definition>(id, name, start_node_id, std::move(nodes));
}

script_runner::script_runner() {

}

bool script_runner::run(std::shared_ptr<script_definition> script) {
	auto& instance = scripts.emplace_back(std::make_unique<running_script>(std::move(script), *this));
	return instance->process_entry_point();
}

void script_runner::clean() {
#ifdef NFWK_CPP_20
	std::erase_if(scripts, [](const auto& script) {
		return script->is_done();
	});
#else
	for (int i{ 0 }; i < static_cast<int>(scripts.size()); i++) {
		if (scripts[i]->is_done()) {
			scripts.erase(scripts.begin() + i);
			i--;
		}
	}
#endif
}

void script_runner::on_interactive_node(const script_node& node) {
	const auto& node_event = node_shown[node.type()];
	node_event.emit(&node);
}

script_manager::script_manager() {
	node_factory = std::make_unique<script_node_factory>();
	loader = std::make_unique<script_loader>(*node_factory);
	runner = std::make_unique<script_runner>();
}

bool script_manager::run(std::string_view id) {
	if (auto definition = find(id)) {
		return runner->run(definition);
	} else {
		warning(scripts::log, "Did not find script: {}", id);
		return false;
	}
}

void script_manager::clean() {
	runner->clean();
	// todo: clean unused definitions.
}

std::shared_ptr<script_definition> script_manager::find(std::string_view id) const {
	ASSERT(is_identifier_normalized(id));
	auto it = std::find_if(definitions.begin(), definitions.end(), [id](const auto& definition) {
		return definition->get_id() == id;
	});
	return it != definitions.end() ? *it : nullptr;
}

void script_manager::load(const std::filesystem::path& path) {
	if (auto definition = loader->load(path)) {
		definitions.emplace_back(std::move(definition));
	} else {
		warning(scripts::log, "Failed to load script: {}",path);
	}
}

script_runner& script_manager::get_runner() {
	return *runner;
}

}
