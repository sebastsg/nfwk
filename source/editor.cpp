#include "editor.hpp"
#include "ui.hpp"
#include "window.hpp"

namespace no::internal {

static void add_open_editor_debug_menu_item() {
	debug::menu::add("nfwk-open-editor", "Editor", [] {
		if (ui::menu_item("Open")) {
			auto state = program_state::current();
			state->change_state<editor_state>();
		}
	});
}

void initialize_editor() {
	add_open_editor_debug_menu_item();
}

}

namespace no {

static std::vector<std::pair<std::string, std::function<std::unique_ptr<abstract_editor>()>>> editor_makers;

editor_state::editor_state() {
	debug::menu::remove("nfwk-open-editor");
	debug::menu::add("nfwk-editor", "Editor", [this] {
		for (const auto& [title, make_editor] : editor_makers) {
			if (ui::menu_item(title)) {
				open(make_editor());
			}
		}
	});
}

editor_state::~editor_state() {
	internal::add_open_editor_debug_menu_item();
	debug::menu::remove("nfwk-editor");
}

void editor_state::update() {
	const auto center = window().size().to<float>() / 2.0f;
	for (int i{ 0 }; i < static_cast<int>(editors.size()); i++) {
		auto& editor = *editors[i];
		ImGui::PushID(&editor);
		bool open{ true };
		constexpr int flags{ ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize };
		if (auto end = ui::push_window(CSTRING(editor.get_title() << "##" << &editor), center, {}, flags, &open)) {
			editor.update();
			if (!open) {
				close(editor);
				i--;
			}
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

void editor_state::draw() {

}

void editor_state::open(std::unique_ptr<abstract_editor> editor) {
	editors.emplace_back(std::move(editor));
}

void editor_state::close(abstract_editor& editor_to_close) {
	std::erase_if(editors, [&](const auto& open_editor) {
		return &editor_to_close == open_editor.get();
	});
}

void register_editor(std::string_view title, std::function<std::unique_ptr<abstract_editor>()> make_editor) {
	editor_makers.emplace_back(title, std::move(make_editor));
}

}
