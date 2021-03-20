#include "scripts/script_editor.hpp"
#include "scripts/script_node.hpp"
#include "graphics/window.hpp"
#include "graphics/surface.hpp"
#include "graphics/ui.hpp"
#include "assets.hpp"
#include "platform.hpp"
#include "random.hpp"
#include "debug_menu.hpp"
#include "imgui_loop_component.hpp"

namespace nfwk {

script_editor::script_editor(editor_state& editor) : abstract_editor{ editor } {
	create_new_script();

	debug::menu::add("nfwk-script-editor", "Script", [this] {
		ui::menu_item("Properties", show_properties);
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
	const auto window_size = editor.window->size().to<float>();
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
	//script.tree.load(id);
	error("scripts", "Not implemented");
	ASSERT(false);
}

void script_editor::save_script() {
	error("scripts", "Not implemented");
	ASSERT(false);
	//script.tree.save();
	//script.dirty = false;
}

void script_editor::update_nodes(vector2f offset) {
	script.hovered_node = std::nullopt;
	if (ImGui::IsMouseClicked(0) && !is_context_menu_open) {
		script.selected_node = std::nullopt;
	}
	for (auto node : script.tree.get_nodes()) {
		ImGui::PushID(node->id);
		const auto real_position = offset + node->transform.position;

		ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);
		ImGui::SetCursorScreenPos(real_position + 8.0f);
		ImGui::BeginGroup();
		ui::text("%s", node->get_name().data());
		ui::inline_next();
		ui::colored_text({ 0.65f, 0.7f, 0.85f }, "#%i", node->id);
		if (node->is_interactive()) {
			ui::rectangle(real_position + vector2f{ node->transform.scale.x - 100.0f, 14.0f }, 6.0f, { 1.0f, 0.5f, 0.5f, 1.0f });
			ImGui::SameLine(node->transform.scale.x - 96.0f);
			ui::colored_text({ 1.0f, 0.5f, 0.5f, 1.0f }, "interactive");
		}
		script.dirty |= node->update_editor();
		ImGui::EndGroup();
		node->transform.scale = vector2f{ ImGui::GetItemRectSize() } + 16.0f;
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);

		if (ImGui::IsMouseHoveringRect(real_position, real_position + node->transform.scale)) {
			script.hovered_node = node->id;
			if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) {
				script.selected_node = node->id;
			}
		}

		if (script.selected_node == node->id && ImGui::IsMouseDragging(0)) {
			node->transform.position += ImGui::GetIO().MouseDelta;
		}

		vector4f node_background_color{ 0.12f, 0.16f, 0.24f, 1.0f };
		vector4f outline_color{ 1.0f, 1.0f, 1.0f, 0.4f };
		if (script.selected_node == node->id) {
			node_background_color.xyz = { 0.16f, 0.2f, 0.28f };
			outline_color = { 1.0f, 1.0f, 1.0f, 0.8f };
		}
		if (script.tree.get_start_node_id() == node->id) {
			if (script.selected_node == node->id) {
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
	auto node = script.selected_node.has_value() ? script.tree.get_node(script.selected_node.value()) : nullptr;
	if (node) {
		update_node_context_menu(*node);
	} else {
		std::unordered_map<std::string_view, std::vector<script_node_constructor>> categories;
		for (const auto& constructor : get_all_script_node_constructors()) {
			categories[constructor.get_category()].push_back(constructor);
		}
		for (const auto& [category, constructors] : categories) {
			if (category.empty()) {
				for (const auto& constructor : constructors) {
					if (ui::menu_item(constructor.get_name())) {
						node = constructor.construct();
					}
				}
			}
		}
		for (const auto& [category, constructors] : categories) {
			if (!category.empty()) {
				if (auto end = ui::menu(category)) {
					for (const auto& constructor : constructors) {
						if (ui::menu_item(constructor.get_name())) {
							node = constructor.construct();
						}
					}
				}
			}
		}
		if (node) {
			node->transform.position = scene_position;
			script.tree.add_node(node);
			script.dirty = true;
		}
	}
	ImGui::EndPopup();
	ImGui::PopStyleVar();
}

void script_editor::update_node_context_menu(script_node& node) {
	if (script.output_from_node.has_value()) {
		auto from_node = script.tree.get_node(script.output_from_node.value());
		bool can_connect{ from_node->id != node.id };
		if (can_connect) {
			if (ImGui::MenuItem("Connect")) {
				from_node->add_output(script.output_slot, node.id);
				script.output_from_node = std::nullopt;
				script.output_slot = std::nullopt;
				script.dirty = true;
			}
		}
		if (ui::menu_item("Cancel connecting")) {
			script.output_from_node = std::nullopt;
			script.output_slot = std::nullopt;
		}
	} else {
		if (node.output_type() == script_node_output_type::variable) {
			if (ui::menu_item("Add output")) {
				script.output_slot = std::nullopt;
				script.output_from_node = script.selected_node;
			}
		} else if (node.output_type() == script_node_output_type::boolean) {
			if (auto end = ui::menu("Set output")) {
				if (ui::menu_item("True")) {
					script.output_slot = 1;
					script.output_from_node = script.selected_node;
				}
				if (ui::menu_item("False")) {
					script.output_slot = 0;
					script.output_from_node = script.selected_node;
				}
			}
		} else if (node.output_type() == script_node_output_type::single) {
			if (ui::menu_item("Set output")) {
				script.output_slot = 0;
				script.output_from_node = script.selected_node;
			}
		} else {
			warning("scripts", "Invalid output type: {}", static_cast<int>(node.output_type()));
		}
		if (node.used_output_slots_count() > 0) {
			if (auto end_delete_links = ui::menu("Delete output to")) {
				for (const auto& output : node.get_outputs()) {
					const auto name = script.tree.get_node(output.to_node())->get_name();
					if (ui::menu_item(STRING(name) + " (" + std::to_string(output.to_node()) + ")")) {
						node.delete_output_slot(output.slot());
					}
				}
			}
		}
	}
	if (node.can_be_entry_point()) {
		if (ui::menu_item("Set as starting point")) {
			script.tree.set_start_node(node.id);
			script.dirty = true;
		}
	}
	if (ui::menu_item("Delete")) {
		for (auto other_node : script.tree.get_nodes()) {
			other_node->delete_output_node(node.id);
		}
		script.tree.delete_node(script.selected_node.value());
		script.selected_node = std::nullopt;
		script.output_from_node = std::nullopt;
		script.output_slot = std::nullopt;
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
	const auto& keyboard = editor.window->keyboard;
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

void script_editor::update_node_link_output(const script_node_output& output, script_node& node, vector2f offset) {
	const auto* out_node = script.tree.get_node(output.to_node());
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
	if (node.output_type() == script_node_output_type::boolean) {
		stroke_color = (output.slot() == 0 ? ImColor{ 200, 100, 100 } : ImColor{ 100, 200, 100 });
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
	while (circles.size() <= node.used_output_slots_count()) {
		circles.emplace_back(bezier_copy[0], 1, 0.0f);
	}
	auto& circle = circles[output.slot()];

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
	for (auto node : script.tree.get_nodes()) {
		for (const auto& output : node->get_outputs()) {
			ASSERT(output.to_node() != -1);
			update_node_link_output(output, *node, offset);
		}
	}
}

}
