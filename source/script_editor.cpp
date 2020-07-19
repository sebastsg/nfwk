#include "script_editor.hpp"
#include "window.hpp"
#include "assets.hpp"
#include "platform.hpp"
#include "surface.hpp"
#include "ui.hpp"
#include "random.hpp"

namespace no {

script_editor::script_editor() {
	create_new_script();

	debug::menu::add("nfwk-script-editor", "Script", [this] {
		if (ui::menu_item("Properties", show_properties)) {
			
		}
		if (ui::menu_item("Save")) {
			save_script();
		}
		ui::separate();
		ui::text(script.tree.name + " / " + script.tree.id);
	});
}

script_editor::~script_editor() {
	
}

void script_editor::update() {
	const auto window_size = program_state::current()->window().size().to<float>();
	vector2f position{ 0.0f, 24.0f };
	vector2f size{ window_size - position };
	if (show_properties) {
		size.x = 320.0f;
		update_properties_window(position, size);
		position.x += size.x;
		size.x = window_size.x - position.x;
	}
	update_nodes_window(position, size);
}

bool script_editor::is_dirty() const {
	return script.dirty;
}

void script_editor::update_properties_window(vector2f position, vector2f size) {
	if (auto end = ui::push_window("##script-properties-window", position, size, ui::background_window_flags)) {
		ui::text("Properties");
		ui::input("Identifier", script.tree.id);
		ui::input("Name", script.tree.name);
		ui::separate();
		if (ui::button("Close")) {
			show_properties = false;
		}
	}
}

void script_editor::update_nodes_window(vector2f position, vector2f size) {
	if (auto end = ui::push_window("##nodes-window", position, size, ui::background_window_flags)) {
		ImGui::BeginGroup();
		ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.235f, 0.235f, 0.275f, 0.78125f });
		ImGui::BeginChild("##node-grid", { 0, 0 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

		const auto offset = vector2f{ ImGui::GetCursorScreenPos() } - script.scrolling;
		ui::grid(offset, 32.0f, { 0.78125f, 0.78125f, 0.78125f, 0.15625f });
		update_node_links(offset);

		ImGui::PushItemWidth(256.0f);
		update_nodes(offset);
		ImGui::PopItemWidth();
		ImGui::GetWindowDrawList()->ChannelsMerge();

		if (ImGui::IsMouseClicked(1)) {
			if (!script.hovered_node.has_value()) {
				script.selected_node = std::nullopt;
			}
			ImGui::OpenPopup("grid-context-menu");
		}

		update_context_menu(offset);
		update_scrolling();

		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::EndGroup();
	}
}

void script_editor::create_new_script() {
	script = {};
	script.tree.id = random_number_generator::global().string(20);
	script.tree.name = "New script";
}

void script_editor::load_script(const std::string& id) {
	script = {};
	script.tree.load(id);
}

void script_editor::save_script() {
	script.tree.save();
	script.dirty = false;
}

void script_editor::update_nodes(vector2f offset) {
	script.hovered_node = std::nullopt;
	if (ImGui::IsMouseClicked(0) && !is_context_menu_open) {
		script.selected_node = std::nullopt;
	}
	for (auto& [node_id, node] : script.tree.nodes) {
		ImGui::PushID(node_id);
		const auto real_position = offset + node->transform.position;

		ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);
		ImGui::SetCursorScreenPos(real_position + 8.0f);
		ImGui::BeginGroup();
		ui::text(node->get_name());
		script.dirty |= node->update_editor();
		ImGui::EndGroup();
		node->transform.scale = vector2f{ ImGui::GetItemRectSize() } + 16.0f;
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);

		if (ImGui::IsMouseHoveringRect(real_position, real_position + node->transform.scale)) {
			script.hovered_node = node_id;
			if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) {
				script.selected_node = node_id;
			}
		}

		if (script.selected_node == node_id && ImGui::IsMouseDragging(0)) {
			node->transform.position += ImGui::GetIO().MouseDelta;
		}

		vector4f node_background_color{ 0.12f, 0.16f, 0.24f, 1.0f };
		vector4f outline_color{ 1.0f, 1.0f, 1.0f, 0.4f };
		if (script.selected_node == node_id) {
			node_background_color.xyz = { 0.16f, 0.2f, 0.28f };
			outline_color = { 1.0f, 1.0f, 1.0f, 0.8f };
		}
		if (script.tree.start_node_id == node->id) {
			if (script.selected_node == node_id) {
				node_background_color.xyz = { 0.25f, 0.45f, 0.3f };
			} else {
				node_background_color.xyz = { 0.25f, 0.45f, 0.3f };
			}
		}
		ui::rectangle(real_position, node->transform.scale, node_background_color);
		ui::outline(real_position, node->transform.scale, outline_color);
		ImGui::PopID();
	}
}

void script_editor::update_context_menu(vector2f offset) {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 8.0f, 8.0f });
	is_context_menu_open = ImGui::BeginPopup("grid-context-menu");
	if (!is_context_menu_open) {
		ImGui::PopStyleVar();
		return;
	}
	const auto scene_position = vector2f{ ImGui::GetMousePosOnOpeningCurrentPopup() } - offset;
	auto node = script.selected_node.has_value() ? script.tree.nodes[script.selected_node.value()] : nullptr;
	if (node) {
		update_node_context_menu(*node);
	} else {
		std::unordered_map<std::string_view, std::vector<const script_node_constructor*>> categories;
		for (const auto& constructor : get_registered_script_nodes()) {
			categories[constructor.category].push_back(&constructor);
		}
		for (const auto& [category, constructors] : categories) {
			if (category.empty()) {
				for (const auto& constructor : constructors) {
					if (ui::menu_item(constructor->name)) {
						node = constructor->constructor();
					}
				}
			}
		}
		for (const auto& [category, constructors] : categories) {
			if (!category.empty()) {
				if (auto end = ui::menu(category)) {
					for (const auto& constructor : constructors) {
						if (ui::menu_item(constructor->name)) {
							node = constructor->constructor();
						}
					}
				}
			}
		}
		if (node) {
			node->id = script.tree.id_counter;
			script.tree.id_counter++;
			node->transform.position = scene_position;
			script.tree.nodes[node->id] = node;
			script.dirty = true;
		}
	}
	ImGui::EndPopup();
	ImGui::PopStyleVar();
}

void script_editor::update_node_context_menu(script_node& node) {
	if (script.node_index_link_from.has_value()) {
		auto from_node = script.tree.nodes[script.node_index_link_from.value()];
		bool can_connect = true;
		if (from_node->id == node.id) {
			can_connect = false;
		}
		if (from_node->type() == message_node::full_type && node.type() == message_node::full_type) {
			can_connect = false;
		}
		if (can_connect && ImGui::MenuItem("Connect")) {
			from_node->set_output_node(script.node_link_from_out, node.id);
			script.node_index_link_from = std::nullopt;
			script.node_link_from_out = std::nullopt;
			script.dirty = true;
		}
		if (ui::menu_item("Cancel connecting")) {
			script.node_index_link_from = std::nullopt;
			script.node_link_from_out = std::nullopt;
		}
	} else {
		if (node.output_type() == node_output_type::variable) {
			if (ui::menu_item("Start link")) {
				script.node_index_link_from = script.selected_node;
			}
		} else {
			if (auto end = ui::menu("Start link")) {
				switch (node.output_type()) {
				case node_output_type::boolean:
					if (ui::menu_item("True")) {
						script.node_link_from_out = 1;
						script.node_index_link_from = script.selected_node;
					}
					if (ui::menu_item("False")) {
						script.node_link_from_out = 0;
						script.node_index_link_from = script.selected_node;
					}
					break;
				case node_output_type::single:
					if (ui::menu_item("Forward")) {
						script.node_link_from_out = 0;
						script.node_index_link_from = script.selected_node;
					}
					break;
				}
			}
		}
	}
	if (node.type() != choice_node::full_type && ui::menu_item("Set as starting point")) {
		script.tree.start_node_id = node.id;
		script.dirty = true;
	}
	if (ui::menu_item("Delete")) {
		for (auto& other_node : script.tree.nodes) {
			other_node.second->remove_output_node(node.id);
		}
		delete script.tree.nodes[script.selected_node.value()];
		script.tree.nodes.erase(script.selected_node.value());
		script.selected_node = std::nullopt;
		script.node_index_link_from = std::nullopt;
		script.node_link_from_out = std::nullopt;
		script.dirty = true;
	}
}

void script_editor::update_scrolling() {
	if (ImGui::IsAnyItemActive()) {
		return;
	}
	if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(2, 0.0f)) {
		script.scrolling.x -= ImGui::GetIO().MouseDelta.x;
		script.scrolling.y -= ImGui::GetIO().MouseDelta.y;
	}
	const float scroll_speed{ 20.0f };
	const auto& keyboard = program_state::current()->keyboard();
	if (keyboard.is_key_down(key::up)) {
		script.scrolling.y -= scroll_speed;
	}
	if (keyboard.is_key_down(key::left)) {
		script.scrolling.x -= scroll_speed;
	}
	if (keyboard.is_key_down(key::down)) {
		script.scrolling.y += scroll_speed;
	}
	if (keyboard.is_key_down(key::right)) {
		script.scrolling.x += scroll_speed;
	}
}

void script_editor::update_node_link_output(const node_output& output, script_node& node, vector2f offset) {
	const auto* out_node = script.tree.nodes[output.node_id];
	const auto from_size = node.transform.scale;
	const auto to_size = out_node->transform.scale;
	auto from_position = node.transform.position;
	auto to_position = out_node->transform.position;
	const auto delta_position = (to_position + to_size / 2.0f) - (from_position + from_size / 2.0f);
	const bool delta_x_greater_than_y{ std::abs(delta_position.x) > std::abs(delta_position.y) };

	bool entirely_left = false;
	bool partially_left = false;
	bool entirely_right = false;
	bool partially_right = false;
	if (from_position.x < to_position.x) {
		partially_left = true;
		if (from_position.x + from_size.x < to_position.x) {
			entirely_left = true;
		}
	} else {
		partially_right = true;
		if (from_position.x > to_position.x + to_size.x) {
			entirely_right = true;
		}
	}

	bool entirely_above = false;
	bool partially_above = false;
	bool entirely_below = false;
	bool partially_below = false;
	if (from_position.y < to_position.y) {
		partially_above = true;
		if (from_position.y + from_size.y < to_position.y) {
			entirely_above = true;
		}
	} else {
		partially_below = true;
		if (from_position.y > to_position.y + to_size.y) {
			entirely_below = true;
		}
	}

	entirely_above = partially_above;
	entirely_below = partially_below;

	float bez_factor{ 50.0f };
	vector2f b1{ bez_factor, 0.0f };
	vector2f b2{ -bez_factor, 0.0f };

	if (entirely_above) {
		if (delta_x_greater_than_y) {
			if (entirely_left) {
				from_position.x += from_size.x;
				from_position.y += from_size.y / 2.0f;
				to_position.y += to_size.y / 2.0f;
			} else if (entirely_right) {
				from_position.y += from_size.y / 2.0f;
				to_position.x += to_size.x;
				to_position.y += to_size.y / 2.0f;
				b1.x = -bez_factor;
				b2.x = bez_factor;
			} else if (partially_left || partially_right) {
				from_position.x += from_size.x / 2.0f;
				from_position.y += from_size.y;
				to_position.x += to_size.x / 2.0f;
				b1 = { 0.0f, bez_factor };
				b2 = { 0.0f, -bez_factor };
			}
		} else {
			from_position.x += from_size.x / 2.0f;
			from_position.y += from_size.y;
			to_position.x += to_size.x / 2.0f;
			b1 = { 0.0f, bez_factor };
			b2 = { 0.0f, -bez_factor };
		}
	} else if (entirely_below) {
		if (delta_x_greater_than_y) {
			if (entirely_left) {
				from_position.x += from_size.x;
				from_position.y += from_size.y / 2.0f;
				to_position.y += to_size.y / 2.0f;
			} else if (entirely_right) {
				from_position.y += from_size.y / 2.0f;
				to_position.x += to_size.x;
				to_position.y += to_size.y / 2.0f;
				b1.x = -bez_factor;
				b2.x = bez_factor;
			} else if (partially_left || partially_right) {
				from_position.x += from_size.x / 2.0f;
				to_position.x += to_size.x / 2.0f;
				to_position.y += to_size.y;
				b1 = { 0.0f, -bez_factor };
				b2 = { 0.0f, bez_factor };
			}
		} else {
			from_position.x += from_size.x / 2.0f;
			to_position.x += to_size.x / 2.0f;
			to_position.y += to_size.y;
			b1 = { 0.0f, -bez_factor };
			b2 = { 0.0f, bez_factor };
		}
	}

	ImU32 stroke_color{ ImColor{ 200, 200, 100 } };
	if (node.output_type() == node_output_type::boolean) {
		stroke_color = (output.out_id == 0 ? ImColor{ 200, 100, 100 } : ImColor{ 100, 200, 100 });
	}

	from_position += offset;
	to_position += offset;

	auto draw_list = ImGui::GetWindowDrawList();
	draw_list->PathLineTo(from_position);
	draw_list->PathBezierCurveTo(from_position + b1, to_position + b2, to_position);
	std::vector<vector2f> bezier_copy;
	for (int path_index{ 0 }; path_index < draw_list->_Path.Size; path_index++) {
		bezier_copy.emplace_back(draw_list->_Path[path_index]);
	}

	auto& circles = node_circles[&node];
	while (circles.size() <= node.out.size()) {
		circles.emplace_back(bezier_copy[0], 1, 0.0f);
	}
	auto& circle = circles[output.out_id];

	draw_list->PathStroke(stroke_color, false, 3.0f + 3.0f * std::abs(circle.pulse - 0.5f));

	// the curve might have changed:
	if (circle.bezier_index >= static_cast<int>(bezier_copy.size())) {
		circle.bezier_index = static_cast<int>(bezier_copy.size()) - 1;
		circle.position = bezier_copy[circle.bezier_index - 1];
	}

	const auto target_position = bezier_copy[circle.bezier_index];
	const auto start_position = circle.bezier_index == 0 ? target_position : bezier_copy[circle.bezier_index - 1];
	circle.position = (1.0f - circle.pulse) * start_position + circle.pulse * target_position;
	circle.pulse += 0.01f;
	if (circle.pulse > 1.0f) {
		circle.pulse = 0.0f;
		circle.bezier_index++;
		if (circle.bezier_index >= static_cast<int>(bezier_copy.size())) {
			circle.position = bezier_copy[0];
			circle.bezier_index = 1;
		}
	}
	draw_list->AddCircleFilled(circle.position, 5.0f + 3.0f * std::abs(circle.pulse - 0.5f), stroke_color);
}

void script_editor::update_node_links(vector2f offset) {
	for (auto& [id, node] : script.tree.nodes) {
		for (const auto& output : node->out) {
			if (output.node_id != -1) {
				update_node_link_output(output, *node, offset);
			}
		}
	}
}

}
