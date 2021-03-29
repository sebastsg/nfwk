#pragma once

#include "editor.hpp"
#include "scripts/script_tree.hpp"
#include "vector2.hpp"

struct ImDrawList;

namespace nfwk {

class script_node_output;

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

	std::unique_ptr<script_tree> tree;
	std::optional<int> output_from_node;
	std::optional<int> output_slot;
	std::optional<int> selected_node;
	std::optional<int> hovered_node;
	bool dirty{ false };
	vector2f scrolling;

};

class script_editor : public abstract_editor {
public:

	static constexpr std::u8string_view title{ u8"Script editor" };

	script_node_factory* node_factory{ nullptr };

	script_editor(editor_container& container);
	~script_editor() override;

	void update() override;
	bool is_dirty() const override;

	std::u8string_view get_title() const override {
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
	void load_script(const std::u8string& id);
	void save_script();

	void update_nodes(vector2f offset);
	void update_context_menu(vector2f offset);
	void update_node_context_menu(script_node& node);
	void update_scrolling();
	void update_node_link_output(const script_node_output& output, script_node& node, vector2f offset);
	void update_node_links(vector2f offset);

};

}
