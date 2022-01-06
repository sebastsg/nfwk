#pragma once

#include "editor.hpp"
#include "scripts/script_tree.hpp"
#include "vector2.hpp"

struct ImDrawList;

namespace nfwk::ui {
class main_menu_bar;
}

namespace nfwk::script {

class node_output;

class link_circle {
public:

	vector2f position;
	int bezier_index{ 0 };
	float pulse{ 0.0f };

	link_circle() = default;

	link_circle(vector2f position, int bezier_index, float pulse)
		: position{ position }, bezier_index{ bezier_index }, pulse{ pulse } {}

};

class script_edit_state {
public:

	std::string id;
	std::string name;
	std::optional<int> start_node_id;
	std::vector<std::shared_ptr<script_node>> nodes;
	
	std::optional<int> output_from_node;
	std::optional<int> output_slot;
	std::optional<int> selected_node;
	std::optional<int> hovered_node;
	bool dirty{ false };
	vector2f scrolling;

};

class script_editor : public ui::window_wrapper {
public:

	script_node_factory* node_factory{ nullptr };

	script_editor(std::optional<std::filesystem::path> path, std::shared_ptr<ui::main_menu_bar> menu_bar);
	script_editor(const script_editor&) = delete;
	script_editor(script_editor&&) = delete;
	
	~script_editor() override;

	script_editor& operator=(const script_editor&) = delete;
	script_editor& operator=(script_editor&&) = delete;
	
	void update(ui::window_container& container) override;
	bool has_unsaved_changes() const override;

	template<typename Node>
	void add_node_ui(const std::function<bool(Node&)>& edit_node_ui) {
		node_ui_functions[Node::full_type].emplace_back([edit_node_ui](script_node& node) {
			return edit_node_ui(static_cast<Node&>(node));
		});
	}

private:

	std::optional<std::filesystem::path> path;
	std::shared_ptr<ui::main_menu_bar> menu_bar;
	
	script_edit_state script;

	bool show_properties{ false };
	bool is_context_menu_open{ false };

	std::unordered_map<const script_node*, std::vector<link_circle>> node_circles;
	std::unordered_map<int, std::vector<std::function<bool(script_node&)>>> node_ui_functions;
	
	void update_nodes_window(vector2f position, vector2f size);
	void update_properties_window(vector2f position, vector2f size);

	void update_nodes(vector2f offset);
	void update_context_menu(vector2f offset);
	void update_node_context_menu(script_node& node);
	void update_scrolling();
	void update_node_link_output(const node_output& output, script_node& node, vector2f offset);
	void update_node_links(vector2f offset);

	void add_core_node_ui();

};

}
