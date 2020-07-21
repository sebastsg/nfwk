#pragma once

#include "editor.hpp"
#include "script.hpp"
#include "draw.hpp"

struct ImDrawList;

namespace no {

struct link_circle {

	vector2f position;
	int bezier_index{ 0 };
	float pulse{ 0.0f };

	link_circle() = default;

	link_circle(vector2f position, int bezier_index, float pulse)
		: position{ position }, bezier_index{ bezier_index }, pulse{ pulse } {
	}

};

struct script_edit_state {

	script_tree tree;
	std::optional<int> output_from_node;
	std::optional<int> output_slot;
	std::optional<int> selected_node;
	std::optional<int> hovered_node;
	bool dirty{ false };
	vector2f scrolling;

};

class script_editor : public abstract_editor {
public:

	static constexpr std::string_view title{ "Script editor" };

	script_editor();
	~script_editor() override;

	void update() override;
	bool is_dirty() const override;

	std::string_view get_title() const override {
		return title;
	}

private:

	bool show_properties{ false };

	std::unordered_map<script_node*, std::vector<link_circle>> node_circles;

	script_edit_state script;

	bool is_context_menu_open{ false };

	void update_nodes_window(vector2f position, vector2f size);
	void update_properties_window(vector2f position, vector2f size);

	void create_new_script();
	void load_script(const std::string& id);
	void save_script();

	void update_nodes(vector2f offset);
	void update_context_menu(vector2f offset);
	void update_node_context_menu(script_node& node);
	void update_scrolling();
	void update_node_link_output(const node_output& output, script_node& node, vector2f offset);
	void update_node_links(vector2f offset);

};

}
