#include "editor.hpp"
#include "graphics/window.hpp"
#include "debug_menu.hpp"
#include "objects/object_editor.hpp"
#include "scripts/script_editor.hpp"
#include "presets.hpp"

namespace nfwk {

abstract_editor::abstract_editor(editor_container& container) : container{ container } {}

editor_container::editor_container() {
	debug::menu::add(u8"nfwk-editor", u8"Editor", [this] {
		for (const auto& [title, make_editor] : editor_makers) {
			if (ui::menu_item(title)) {
				open(make_editor());
			}
		}
	});
	register_editor<object_class_editor>();
	register_editor<object_class_list_editor>();
	register_editor<script_editor>();
}

editor_container::~editor_container() {
	debug::menu::remove(u8"nfwk-editor");
}

void editor_container::update() {
	if (auto viewport = ImGui::GetMainViewport()) {
		vector2f position{ viewport->Pos.x, viewport->Pos.y };
		position.y += 24.0f;
		vector2f size{ viewport->Size.x, viewport->Size.y };
		ImGui::SetNextWindowPos(position);
		ImGui::SetNextWindowSize(size - position);
		ImGui::SetNextWindowViewport(viewport->ID);
	}
	ImGuiWindowFlags window_flags{ ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking };
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("##editor-root-window", nullptr, window_flags);
	ImGui::PopStyleVar(3);
	ImGui::DockSpace(ImGui::GetID("editor-dock"), { 0.0f, 0.0f }, ImGuiDockNodeFlags_NoCloseButton);
	dock_node = ImGui::DockBuilderGetNode(ImGui::GetID("editor-dock"));
	for (int i{ 0 }; i < static_cast<int>(editors.size()); i++) {
		auto& editor = *editors[i];
		ImGui::PushID(&editor);
		editor.update();
		if (!editor.is_open()) {
			close(editor);
			i--;
		}
		ImGui::PopID();
	}
	ImGui::End();
	bool dirty{ false };
	for (const auto& editor : editors) {
		dirty |= editor->is_dirty();
	}
	ui::text(u8"Saved: %s", dirty ? u8"No " : u8"Yes");
	ui::inline_next();
	vector4f saved{ 0.2f, 1.0f, 0.4f, 0.7f };
	vector4f unsaved{ 0.8f, 0.8f, 0.1f, 0.7f };
	vector4f status{ dirty ? unsaved : saved };
	vector4f border{ status + vector4f{ 0.2f, 0.2f, 0.2f, 0.0f } };
	//ui::image(*blank_texture, 10.0f, 0.0f, 1.0f, status, border);
}

void editor_container::dock(int direction, float ratio) {
	auto window = ImGui::GetCurrentWindow();
	const auto node = direction == ImGuiDir_None ? dock_node->CentralNode : dock_node;
	if (window->DockNode && window->DockNode->ID == node->ID) {
		warning(ui::log, u8"Window is already docked: {}", window->Name);
		return;
	}
	ImGui::DockContextQueueDock(ImGui::GetCurrentContext(), dock_node->HostWindow, node, window, direction, ratio, false);
}

void editor_container::open(std::unique_ptr<abstract_editor> editor) {
	editors.emplace_back(std::move(editor));
}

void editor_container::close(abstract_editor& editor_to_close) {
#ifndef NFWK_CPP_20
	for (int i{ 0 }; i < static_cast<int>(editors.size()); i++) {
		if (&editor_to_close == editors[i].get()) {
			editors.erase(editors.begin() + i);
			i--;
		}
	}
#else
	std::erase_if(editors, [&](const auto& open_editor) {
		return &editor_to_close == open_editor.get();
	});
#endif
}

void editor_container::register_editor(std::u8string_view title, std::function<std::unique_ptr<abstract_editor>()> make_editor) {
	editor_makers.emplace_back(title, std::move(make_editor));
}

}
