#pragma once

#include "event.hpp"

#include <optional>

#include "objects/objects.hpp"

namespace nfwk {
class io_stream;
}

namespace nfwk::script {

class script_runner;
class variable_registry;
class script_node;
class script_node_output;
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
	std::optional<int> scope_id; // mostly used for variable nodes.

	script_context() = default;
	script_context(const script_context&) = delete;
	script_context(script_context&&) = delete;

	~script_context() = default;

	script_context& operator=(const script_context&) = delete;
	script_context& operator=(script_context&&) = delete;
	
};

class script_definition {
public:

	static constexpr std::string_view file_extension{ ".nfwk-script" };

	script_definition(std::string id, std::string name, std::optional<int> start_node_id, std::vector<std::shared_ptr<script_node>> nodes);
	script_definition(const script_definition&) = delete;
	script_definition(script_definition&&) = delete;

	~script_definition() = default;

	script_definition& operator=(const script_definition&) = delete;
	script_definition& operator=(script_definition&&) = delete;
	
	void write(io_stream& stream) const;

	std::shared_ptr<script_node> get_node(int node_id);
	void set_start_node(std::optional<int> node_id);
	std::optional<int> get_start_node_id() const;
	void delete_node(int node_id);
	void add_node(std::shared_ptr<script_node> node);
	const std::vector<std::shared_ptr<script_node>>& get_nodes() const;

	std::string_view get_id() const {
		return id;
	}

	std::string_view get_name() const {
		return name;
	}

private:
	
	void rebuild_valid_nodes();

	std::string id;
	std::string name;
	std::optional<int> start_node_id;
	std::vector<std::shared_ptr<script_node>> nodes;
	std::vector<std::shared_ptr<script_node>> valid_nodes;

};

class running_script {
public:

	script_context context;

	running_script(std::shared_ptr<script_definition> script, script_runner& runner);
	running_script(const running_script&) = delete;
	running_script(running_script&&) = delete;

	~running_script() = default;

	running_script& operator=(const running_script&) = delete;
	running_script& operator=(running_script&&) = delete;
	
	std::optional<int> get_current_node_id() const;
	bool is_done() const;

	[[nodiscard]] bool process_entry_point();
	[[nodiscard]] bool process_output(int node_id, int slot);
	[[nodiscard]] bool process_outputs(int node_id);

private:

	bool process_next_node();

	std::shared_ptr<script_definition> script;
	script_runner& runner;
	std::optional<int> current_node_id;
	bool event_fired{ false };
	bool done{ false };
	
};

class script_loader {
public:

	script_loader(const script_node_factory& factory);
	script_loader(const script_loader&) = delete;
	script_loader(script_loader&&) = delete;

	~script_loader() = default;

	script_loader& operator=(const script_loader&) = delete;
	script_loader& operator=(script_loader&&) = delete;

	std::shared_ptr<script_definition> load(const std::filesystem::path& path) const;

private:

	const script_node_factory& node_factory;
	
};

class script_runner {
public:

	friend class running_script; // to call on_interactive_node()

	script_runner();
	script_runner(const script_runner&) = delete;
	script_runner(script_runner&&) = delete;

	~script_runner() = default;

	script_runner& operator=(const script_runner&) = delete;
	script_runner& operator=(script_runner&&) = delete;
	
	bool run(std::shared_ptr<script_definition> script);
	void clean();

	template<typename Node>
	[[nodiscard]] event_listener listen_to_interactive_node(const std::function<void(const Node&)>& handler) {
		return node_shown[Node::full_type].listen([handler](const script_node& base_node) {
			if (const auto& node = static_cast<const Node&>(base_node)) {
				handler(node);
			}
		});
	}

private:

	void on_interactive_node(const script_node& node);
	
	std::vector<std::unique_ptr<running_script>> scripts;
	std::unordered_map<int, event<const script_node*>> node_shown;
	
};

class script_manager {
public:

	script_manager();
	script_manager(const script_manager&) = delete;
	script_manager(script_manager&&) = delete;

	~script_manager() = default;

	script_manager& operator=(const script_manager&) = delete;
	script_manager& operator=(script_manager&&) = delete;

	bool run(std::string_view id);
	void clean();

	std::shared_ptr<script_definition> find(std::string_view id) const;
	void load(const std::filesystem::path& path);

	script_runner& get_runner();

private:

	std::unique_ptr<script_loader> loader;
	std::unique_ptr<script_runner> runner;
	std::unique_ptr<script_node_factory> node_factory;
	std::vector<std::shared_ptr<script_definition>> definitions;

};

}
