#include "scripts/script_editor.hpp"
#include "scripts/script_node.hpp"
#include "graphics/window.hpp"
#include "graphics/ui.hpp"
#include "assets.hpp"
#include "random.hpp"
#include "debug_menu.hpp"
#include "node_ui.hpp"

namespace nfwk::script {

script_editor::script_editor(std::optional<std::filesystem::path> path_, std::shared_ptr<ui::main_menu_bar> menu_bar_)
	: path{ std::move(path_) }, menu_bar{ std::move(menu_bar_) } {
	if (path.has_value()) {
		script_node_factory node_factory; // todo: get the proper node factory or loader (to allow user nodes)
		script_loader loader{ node_factory };
		if (const auto& definition = loader.load(path.value())) {
			script.id = definition->get_id();
			script.name = definition->get_name();
			script.start_node_id = definition->get_start_node_id();
			script.nodes = definition->get_nodes();
		} else {
			warning(scripts::log, "Failed to load script: {}", path.value());
		}
	}
	if (script.id.empty()) {
		script.id = to_normalized_identifier(random_number_generator::any().string(20));
		script.name = "New script";
	}
	add_core_node_ui();
	if (menu_bar) {
		menu_bar->add(ui::make_id(this), "Script", [this] {
			ui::checkable_menu_item("Properties", show_properties);
			if (ui::menu_item("Save")) {

			}
			ui::separate();
			ui::text("%s / %s", script.name.c_str(), script.id.c_str());
		});
	}
}

script_editor::~script_editor() {
	if (menu_bar) {
		menu_bar->remove(ui::make_id(this));
	}
}

void script_editor::update(ui::window_container& container) {
	const auto* viewport = ImGui::GetWindowViewport();
	const vector2f window_size{ viewport->Size };
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

bool script_editor::has_unsaved_changes() const {
	return script.dirty;
}

void script_editor::update_properties_window(vector2f position, vector2f size) {
	// todo: window id based on script id
	if (auto _ = ui::window("##properties", position, size, ui::background_window_flags)) {
		ui::text("Properties");
		ui::input("Identifier", script.id);
		ui::input("Name", script.name);
		ui::separate();
		if (ui::button("Close")) {
			show_properties = false;
		}
	}
}

void script_editor::update_nodes_window(vector2f position, vector2f size) {
	if (auto _ = ui::window("##nodes-window", position, size, ui::background_window_flags)) {
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

void script_editor::update_nodes(vector2f offset) {
	script.hovered_node = std::nullopt;
	if (ImGui::IsMouseClicked(0) && !is_context_menu_open) {
		script.selected_node = std::nullopt;
	}
	for (auto& node : script.nodes) {
		ImGui::PushID(node->get_id().value_or(-1));
		const auto real_position = offset + node->transform.position;

		ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);
		ImGui::SetCursorScreenPos(real_position + 8.0f);
		ImGui::BeginGroup();
		ui::text("%s", node->get_name().data());
		ui::inline_next();
		ui::colored_text({ 0.65f, 0.7f, 0.85f }, "#%i", node->get_id().value_or(-1));
		if (node->is_interactive()) {
			ui::rectangle(real_position + vector2f{ node->transform.scale.x - 100.0f, 14.0f }, 6.0f, { 1.0f, 0.5f, 0.5f, 1.0f });
			ImGui::SameLine(node->transform.scale.x - 96.0f);
			ui::colored_text({ 1.0f, 0.5f, 0.5f, 1.0f }, "interactive");
		}
		for (const auto& edit_node_ui : node_ui_functions[node->type()]) {
			script.dirty |= edit_node_ui(*node);
		}
		ImGui::EndGroup();
		node->transform.scale = vector2f{ ImGui::GetItemRectSize() } + 16.0f;
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);

		if (ImGui::IsMouseHoveringRect(real_position, real_position + node->transform.scale)) {
			script.hovered_node = node->get_id();
			if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) {
				script.selected_node = node->get_id();
			}
		}

		if (script.selected_node == node->get_id() && ImGui::IsMouseDragging(0)) {
			node->transform.position += ImGui::GetIO().MouseDelta;
		}

		vector4f node_background_color{ 0.12f, 0.16f, 0.24f, 1.0f };
		vector4f outline_color{ 1.0f, 1.0f, 1.0f, 0.4f };
		if (script.selected_node == node->get_id()) {
			node_background_color = { 0.16f, 0.2f, 0.28f, 1.0f };
			outline_color = { 1.0f, 1.0f, 1.0f, 0.8f };
		}
		if (script.start_node_id == node->get_id()) {
			if (script.selected_node == node->get_id()) {
				node_background_color = { 0.25f, 0.45f, 0.3f, 1.0 };
			} else {
				node_background_color = { 0.25f, 0.45f, 0.3f, 1.0f };
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
	auto node = script.selected_node.has_value() ? script.nodes[script.selected_node.value()] : nullptr;
	if (node) {
		update_node_context_menu(*node);
	} else {
		std::unordered_map<std::string_view, std::vector<script_node_constructor>> categories;
		for (const auto& constructor : node_factory->get_all_constructors()) {
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
			script.nodes.push_back(node);
			script.dirty = true;
		}
	}
	ImGui::EndPopup();
	ImGui::PopStyleVar();
}

void script_editor::update_node_context_menu(script_node& node) {
	if (script.output_from_node.has_value()) {
		auto from_node = script.nodes[script.output_from_node.value()];
		bool can_connect{ from_node->get_id() != node.get_id() };
		if (can_connect) {
			if (ui::menu_item("Connect")) {
				from_node->add_output(script.output_slot, node.get_id().value());
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
		if (node.get_output_type() == output_type::variable) {
			if (ui::menu_item("Add output")) {
				script.output_slot = std::nullopt;
				script.output_from_node = script.selected_node;
			}
		} else if (node.get_output_type() == output_type::boolean) {
			if (auto end = ui::menu("Set output")) {
				if (ui::menu_item("true")) {
					script.output_slot = 1;
					script.output_from_node = script.selected_node;
				}
				if (ui::menu_item("false")) {
					script.output_slot = 0;
					script.output_from_node = script.selected_node;
				}
			}
		} else if (node.get_output_type() == output_type::single) {
			if (ui::menu_item("Set output")) {
				script.output_slot = 0;
				script.output_from_node = script.selected_node;
			}
		} else {
			warning(scripts::log, "Invalid output type: {}", static_cast<int>(node.get_output_type()));
		}
		if (node.used_output_slots_count() > 0) {
			if (auto end_delete_links = ui::menu("Delete output to")) {
				for (const auto& output : node.get_outputs()) {
					const auto name = script.nodes[output.to_node()]->get_name();
					std::string text{ name };
					text += " (" + std::to_string(output.to_node()) + ")";
					if (ui::menu_item(text)) {
						node.delete_output_slot(output.slot());
					}
				}
			}
		}
	}
	if (node.can_be_entry_point()) {
		if (ui::menu_item("Set as starting point")) {
			script.start_node_id = node.get_id();
			script.dirty = true;
		}
	}
	if (ui::menu_item("Delete")) {
		for (const auto& other_node : script.nodes) {
			other_node->delete_output_node(node.get_id().value());
		}
		script.nodes[script.selected_node.value()] = nullptr;
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
	//const float scroll_speed{ 20.0f };
	//const auto& keyboard = editor.window->keyboard;
	//if (keyboard.is_key_down(key::up)) {
	//	script.scrolling.y -= scroll_speed;
	//}
	//if (keyboard.is_key_down(key::left)) {
	//	script.scrolling.x -= scroll_speed;
	//}
	//if (keyboard.is_key_down(key::down)) {
	//	script.scrolling.y += scroll_speed;
	//}
	//if (keyboard.is_key_down(key::right)) {
	//	script.scrolling.x += scroll_speed;
	//}
}

void script_editor::update_node_link_output(const node_output& output, script_node& node, vector2f offset) {
	const auto& out_node = script.nodes[output.to_node()];
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
	if (node.get_output_type() == output_type::boolean) {
		stroke_color = output.slot() == 0 ? ImColor{ 200, 100, 100 } : ImColor{ 100, 200, 100 };
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
	while (static_cast<int>(circles.size()) <= node.used_output_slots_count()) {
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
	for (const auto& node : script.nodes) {
		for (const auto& output : node->get_outputs()) {
			ASSERT(output.to_node() != -1);
			update_node_link_output(output, *node, offset);
		}
	}
}

void script_editor::add_core_node_ui() {
	add_node_ui<message_node>(message_node_update_editor);
	add_node_ui<choice_node>(choice_node_update_editor);
	add_node_ui<compare_variable_node>(compare_variable_node_update_editor);
	add_node_ui<modify_variable_node>(modify_variable_node_update_editor);
	add_node_ui<create_variable_node>(create_variable_node_update_editor);
	add_node_ui<variable_exists_node>(variable_exists_node_update_editor);
	add_node_ui<delete_variable_node>(delete_variable_node_update_editor);
	add_node_ui<random_output_node>(random_output_node_update_editor);
	add_node_ui<random_condition_node>(random_condition_node_update_editor);
	add_node_ui<execute_script_node>(execute_script_node_update_editor);
	add_node_ui<trigger_event_node>(trigger_event_node_update_editor);
	add_node_ui<spawn_object_node>(spawn_object_node_update_editor);
}

}
