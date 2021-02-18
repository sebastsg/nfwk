module;

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "assert.hpp"

export module nfwk.ui:script_editor;

import std.core;
import std.filesystem;
import nfwk.core;
import nfwk.scripts;
import nfwk.graphics;
import nfwk.draw;
import nfwk.random;
import nfwk.assets;
import :editor;
import :imgui_wrapper;
import :debug_menu;

struct ImDrawList;

export namespace nfwk {

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

	script_tree tree;
	std::optional<int> output_from_node;
	std::optional<int> output_slot;
	std::optional<int> selected_node;
	std::optional<int> hovered_node;
	bool dirty{ false };
	vector2f position;
	std::vector<game_event*> attached_to_events;

};

class script_editor : public abstract_editor {
public:

	static constexpr std::string_view title{ "Script" };

	script_editor() {
		create_new_script();

#if 0
		debug::menu::add("nfwk-script-editor", "Script", [this] {
			ui::menu_item("Properties", show_properties);
			if (ui::menu_item("Save")) {
				save_script();
			}
			if (auto end = ui::menu("Load")) {
				for (const auto& path : entries_in_directory(asset_path("scripts"), entry_inclusion::only_files, false)) {
					if (path.extension().string() == script_tree::file_extension) {
						const auto load_script_id = path.stem().string();
						if (ui::menu_item(load_script_id)) {
							load_script(load_script_id);
						}
					}
				}
			}
			ui::separate();
			ui::text(script.tree.name + " / " + script.tree.id);
		});
		mouse_scroll_event = program_state::current()->mouse().scroll.listen([this](int delta) {
			const float factor{ static_cast<float>(delta) * 0.05f };
			node_zoom += factor;
			if (node_zoom < 0.1f) {
				node_zoom = 0.1f;
			} else if (node_zoom > 4.0f) {
				node_zoom = 4.0f;
			} else {
				auto state = program_state::current();
				auto mouse_ratio = (state->mouse().position().to<float>() - nodes_window_position) / nodes_window_size - 0.5f;
				mouse_ratio *= 2.0f;
				auto scroll_delta = nodes_window_size * mouse_ratio * factor;
				script.position += scroll_delta;
			}
		});
#endif
	}

	~script_editor() override {
		debug::menu::remove("nfwk-script-editor");
	}

	void update() override {
		update_properties_window();
		update_nodes_window();
	}

	bool is_dirty() const override {
		return script.dirty;
	}


private:

	bool show_properties{ false };
	bool properties_docked{ false };
	bool main_docked{ false };

	std::unordered_map<script_node*, std::vector<link_circle>> node_circles;

	script_edit_state script;

	bool is_context_menu_open{ false };

	float node_zoom{ 1.0f };
	event_listener mouse_scroll_event;
	vector2f nodes_window_position;
	vector2f nodes_window_size;

	int selected_events_index{ 0 };
	game_event* listen_event{ nullptr };
	ui::search_input<game_event*> event_search;

	void update_nodes_window() {
		// script.tree.name + "###script" + this
		if (auto end = ui::window(script.tree.name + "###script", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, &open)) {
			if (auto editor = static_cast<editor_state*>(program_state::current()); editor && !main_docked) {
				editor->dock(ImGuiDir_None, 0.0f);
				main_docked = true;
			}

			//ImGui::SetWindowFontScale(node_zoom);
			//ui::set_zoom(node_zoom);

			ImGui::BeginGroup();
			ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.235f, 0.235f, 0.275f, 0.78125f });
			ImGui::BeginChild("##node-grid", { 0, 0 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse);

			const auto& cursorScreenPosition = ImGui::GetCursorScreenPos();
			const auto offset = vector2f{ cursorScreenPosition.x, cursorScreenPosition.y } - script.position;
			ui::grid(offset, 32.0f * node_zoom, { 0.78125f, 0.78125f, 0.78125f, 0.15625f });
			update_node_links(offset);

			ImGui::PushItemWidth(256.0f * node_zoom);
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

			//ui::set_zoom(1.0f);
		}
	}

	void update_properties_window() {
		// TODO: add 'this'
		if (auto end = ui::window("Properties (" + script.tree.name + ")###properties"/* << this*/, 0, &show_properties)) {
			if (auto editor = static_cast<editor_state*>(program_state::current()); editor && !properties_docked) {
				editor->dock(ImGuiDir_Left, 0.2f);
				properties_docked = true;
			}
			ui::input("Identifier", script.tree.id);
			ui::input("Name", script.tree.name);
			update_events();
		}
	}

	void update_events() {
		ui::separate();
		ui::text("Events");

		script.attached_to_events = game_event_container::global().with_script(script.tree.id);
		for (const auto& class_ : objects::get_classes()) {
			script.attached_to_events = merge_vectors(script.attached_to_events, class_->events.with_script(script.tree.id));
		}

		if (const auto new_index = ui::list("##events", vector_to_strings(script.attached_to_events), selected_events_index, 5)) {
			selected_events_index = new_index.value();
		}
		if (!script.attached_to_events.empty()) {
			ui::inline_next();
			if (ui::button("Stop listening")) {
				auto selected_event = script.attached_to_events[selected_events_index];
				selected_event->detach_script(script.tree.id);
				if (selected_events_index >= static_cast<int>(script.attached_to_events.size())) {
					selected_events_index--;
				}
			}
		}

		ui::text("Listen to");
		ui::inline_next();
		auto selected_event = event_search.update([&](std::string search_term, int limit) {
			search_term = normalized_identifier(search_term);
			std::vector<game_event*> result{ game_event_container::global().search(search_term, limit) };
			for (const auto& class_ : objects::get_classes()) {
				result = merge_vectors(result, class_->events.search(search_term, limit));
				if (static_cast<int>(result.size()) >= limit) {
					break;
				}
			}
			return result;
		});
		if (selected_event.has_value()) {
			listen_event = selected_event.value();
		}
		if (auto disabled = ui::disable_if(!listen_event || (listen_event && listen_event->has_script(script.tree.id)))) {
			if (ui::button("Listen")) {
				if (listen_event) {
					listen_event->attach_script(script.tree.id);
					listen_event = nullptr;
					event_search.reset();
				}
			}
		}
	}

	void create_new_script() {
		script = {};
		script.tree.id = random_number_generator::global().string(20);
		script.tree.name = "New script";
	}

	void load_script(const std::string& id) {
		event_search.reset();
		selected_events_index = 0;
		listen_event = nullptr;
		script = {};
		script.tree.load(id);
	}

	void save_script() {
		script.tree.save();
		script.dirty = false;
	}

	void update_nodes(vector2f offset) {
#if 0
		script.hovered_node = std::nullopt;
		if (ImGui::IsMouseClicked(0) && !is_context_menu_open) {
			script.selected_node = std::nullopt;
		}
		for (auto node : script.tree.get_nodes()) {
			ImGui::PushID(node->id);
			const auto real_position = offset + node->transform.position * node_zoom;

			ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);
			ImGui::SetCursorScreenPos(real_position + 8.0f * node_zoom);
			ImGui::BeginGroup();
			ui::text("%s", node->get_name().data());
			ui::inline_next();
			ui::colored_text({ 0.65f, 0.7f, 0.85f }, "#%i", node->id);
			if (node->is_interactive()) {
				ui::rectangle(real_position + vector2f{ 200.0f, 14.0f } *node_zoom, 6.0f * node_zoom, { 1.0f, 0.5f, 0.5f, 1.0f });
				ImGui::SameLine(204.0f * node_zoom);
				ui::colored_text({ 1.0f, 0.5f, 0.5f, 1.0f }, "interactive");
			}
			script.dirty |= node->update_editor();
			ImGui::EndGroup();
			node->transform.scale = vector2f{ ImGui::GetItemRectSize() } + 16.0f * node_zoom;
			ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);

			vector4f outline_color{ 1.0f, 1.0f, 1.0f, 0.4f };
			if (ImGui::IsMouseHoveringRect(real_position, real_position + node->transform.scale)) {
				outline_color.w = 0.6f;
				script.hovered_node = node->id;
				if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) {
					script.selected_node = node->id;
				}
			}

			if (script.selected_node == node->id && ImGui::IsMouseDragging(0)) {
				node->transform.position += vector2f{ ImGui::GetIO().MouseDelta } / node_zoom;
			}

			vector4f node_background_color{ 0.12f, 0.16f, 0.24f, 1.0f };
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
#endif
	}

	void update_context_menu(vector2f offset) {
#if 0
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
#endif
	}

	void update_node_context_menu(script_node& node) {
#if 0
		if (script.output_from_node.has_value()) {
			auto from_node = script.tree.get_node(script.output_from_node.value());
			bool can_connect{ from_node->id != node.id };
			if (can_connect) {
				if (ui::menu_item("Connect")) {
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
						if (ui::menu_item(name + " (" + std::to_string(output.to_node()) + ")")) {
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
#endif
	}

	void update_scrolling() {
#if 0
		if (ImGui::IsWindowHovered() && !script.hovered_node.has_value()) {
			if (ImGui::IsMouseDragging(2, 0.0f) || ImGui::IsMouseDragging(0, 0.0f)) {
				script.position -= ImGui::GetIO().MouseDelta;
			}
		}
		if (!ImGui::IsAnyItemActive()) {
			const float scroll_speed{ 20.0f };
			const auto& keyboard = program_state::current()->keyboard();
			if (keyboard.is_key_down(key::up)) {
				script.position.y -= scroll_speed;
			}
			if (keyboard.is_key_down(key::left)) {
				script.position.x -= scroll_speed;
			}
			if (keyboard.is_key_down(key::down)) {
				script.position.y += scroll_speed;
			}
			if (keyboard.is_key_down(key::right)) {
				script.position.x += scroll_speed;
			}
		}
#endif
	}

	void update_node_link_output(const script_node_output& output, script_node& node, vector2f offset) {
#if 0
		const auto* out_node = script.tree.get_node(output.to_node());
		const auto from_size = node.transform.scale;
		const auto to_size = out_node->transform.scale;
		auto from_position = node.transform.position * node_zoom;
		auto to_position = out_node->transform.position * node_zoom;
		const auto delta_position = (to_position + to_size / 2.0f) - (from_position + from_size / 2.0f);
		const bool delta_x_greater_than_y{ std::abs(delta_position.x) > std::abs(delta_position.y) };

		bool entirely_left{ false };
		bool partially_left{ false };
		bool entirely_right{ false };
		bool partially_right{ false };
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

		bool entirely_above{ false };
		bool partially_above{ false };
		bool entirely_below{ false };
		bool partially_below{ false };
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

		float bez_factor{ 50.0f * node_zoom };
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
			stroke_color = output.slot() == 0 ? ImColor{ 200, 100, 100 } : ImColor{ 100, 200, 100 };
		}

		from_position += offset;
		to_position += offset;

		auto draw_list = ImGui::GetWindowDrawList();
		draw_list->PathLineTo(from_position);
		draw_list->PathBezierCurveTo(from_position + b1, to_position + b2, to_position);
		if (draw_list->_Path.Size < 2) {
			return;
		}
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

#endif
	}

	void update_node_links(vector2f offset) {
		for (auto node : script.tree.get_nodes()) {
			for (const auto& output : node->get_outputs()) {
				ASSERT(output.to_node() != -1);
				update_node_link_output(output, *node, offset);
			}
		}
	}

};

}
