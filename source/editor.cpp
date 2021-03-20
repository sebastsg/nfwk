#include "editor.hpp"
#include "graphics/ui.hpp"
#include "graphics/window.hpp"
#include "debug_menu.hpp"
#include "imgui_loop_component.hpp"

#include "objects/object_editor.hpp"
#include "scripts/script_editor.hpp"

#include "presets.hpp"

namespace nfwk {

editor_state::editor_state(loop& loop) : subprogram{ loop } {
	window = make_window(*this, "nfwk editor");
	//imgui = std::make_unique<nfwk::imgui_instance>(get_loop(), *window, *window_manager->get_render_context());

	debug::menu::remove("nfwk-open-editor");
	debug::menu::add("nfwk-editor", "Editor", [this] {
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

editor_state::~editor_state() {
	debug::menu::remove("nfwk-editor");
	debug::menu::add("nfwk-open-editor", "Editor", [] {
		//if (ui::menu_item("Open")) {
		//	loop::active_subprogram()->change<editor_state>();
		//}
	});
}

void editor_state::update() {
	const auto center = window->size().to<float>() / 2.0f;
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
	bool dirty{ false };
	for (const auto& editor : editors) {
		dirty |= editor->is_dirty();
	}
	//ui::text("Saved: %s", dirty ? "No " : "Yes");
	//ui::inline_next();
	//vector4f saved{ 0.2f, 1.0f, 0.4f, 0.7f };
	//vector4f unsaved{ 0.8f, 0.8f, 0.1f, 0.7f };
	//vector4f status{ script.dirty ? unsaved : saved };
	//vector4f border{ status + vector4f{ 0.2f, 0.2f, 0.2f, 0.0f } };
	//ImGui::Image((ImTextureID)blank_texture, { 10.0f }, { 0.0f }, { 1.0f }, status, border);
}

void editor_state::open(std::unique_ptr<abstract_editor> editor) {
	editors.emplace_back(std::move(editor));
}

void editor_state::close(abstract_editor& editor_to_close) {
	for (int i{ 0 }; i < static_cast<int>(editors.size()); i++) {
		if (&editor_to_close == editors[i].get()) {
			editors.erase(editors.begin() + i);
			i--;
		}
	}
}

void editor_state::register_editor(std::string_view title, std::function<std::unique_ptr<abstract_editor>()> make_editor) {
	editor_makers.emplace_back(title, std::move(make_editor));
}

}
