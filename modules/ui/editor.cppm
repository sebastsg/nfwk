module;

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

export module nfwk.ui:editor;

import std.core;
import std.memory;
import nfwk.core;
import :imgui_wrapper;

namespace nfwk {

class abstract_editor;

struct editor_maker {
	std::string title;
	std::function<std::unique_ptr<abstract_editor>()> construct;
};

std::vector<editor_maker> editor_makers;

void add_open_editor_debug_menu_item() {
	/*debug::menu::add("nfwk-open-editor", "Editor", [] {
		if (ui::menu_item("Open")) {
			auto state = program_state::current();
			state->change_state<editor_state>();
		}
	});*/
}

void initialize_editor() {
	add_open_editor_debug_menu_item();
}

}

export namespace nfwk {

class abstract_editor {
public:

	abstract_editor() = default;
	abstract_editor(const abstract_editor&) = delete;
	abstract_editor(abstract_editor&&) = default;

	virtual ~abstract_editor() = default;

	abstract_editor& operator=(const abstract_editor&) = delete;
	abstract_editor& operator=(abstract_editor&&) = default;

	virtual void update() = 0;
	virtual bool is_dirty() const = 0;

	bool is_open() const {
		return open;
	}

protected:

	bool open{ true };

};

#if 0
class editor_state : public program_state {
public:

	editor_state() {
		/*debug::menu::remove("nfwk-open-editor");
		debug::menu::add("nfwk-editor", "Editor", [this] {
			for (const auto& editor_maker : editor_makers) {
				if (ui::menu_item(editor_maker.title)) {
					open(editor_maker.construct());
				}
			}
		});*/
	}

	~editor_state() override {
		add_open_editor_debug_menu_item();
		//debug::menu::remove("nfwk-editor");
	}

	void update() override {
		if (auto viewport = ImGui::GetMainViewport()) {
			vector2f position{ viewport->Pos.x, viewport->Pos.y };
			position.y += 24.0f;
			vector2f size{ viewport->Size.x, viewport->Size.y };
			ImGui::SetNextWindowPos(ImVec2{ position.x, position.y });
			ImGui::SetNextWindowSize(ImVec2{ size.x - position.x, size.y - position.y });
			ImGui::SetNextWindowViewport(viewport->ID);
		}
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("##editor-root-window", nullptr, window_flags);
		ImGui::PopStyleVar(3);
		ImGui::DockSpace(ImGui::GetID("editor-dock"), ImVec2{ 0.0f, 0.0f }, ImGuiDockNodeFlags_NoCloseButton);
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
		//ui::text("Saved: %s", dirty ? "No " : "Yes");
		//ui::inline_next();
		//vector4f saved{ 0.2f, 1.0f, 0.4f, 0.7f };
		//vector4f unsaved{ 0.8f, 0.8f, 0.1f, 0.7f };
		//vector4f status{ script.dirty ? unsaved : saved };
		//vector4f border{ status + vector4f{ 0.2f, 0.2f, 0.2f, 0.0f } };
		//ImGui::Image((ImTextureID)blank_texture, { 10.0f }, { 0.0f }, { 1.0f }, status, border);
	}

	void draw() override {

	}

	void open(std::unique_ptr<abstract_editor> editor) {
		editors.emplace_back(std::move(editor));
	}
	void close(abstract_editor& editor_to_close) {
		std::erase_if(editors, [&](const auto& open_editor) {
			return &editor_to_close == open_editor.get();
		});
	}

	void dock(int direction, float ratio) {
		auto window = ImGui::GetCurrentWindow();
		auto node = direction == ImGuiDir_None ? dock_node->CentralNode : dock_node;
		if (window->DockNode && window->DockNode->ID == node->ID) {
			warning("ui", "Window is already docked: {}", window->Name);
			return;
		}
		ImGui::DockContextQueueDock(ImGui::GetCurrentContext(), dock_node->HostWindow, node, window, direction, ratio, false);
	}

private:

	std::vector<std::unique_ptr<abstract_editor>> editors;
	ImGuiWindow* root_window{ nullptr };
	ImGuiDockNode* dock_node{ nullptr };

};
#endif

void register_editor(std::string_view title, std::function<std::unique_ptr<abstract_editor>()> make_editor) {
	editor_makers.emplace_back(editor_maker{ std::string{ title }, std::move(make_editor) });
}

template<typename Editor>
void register_editor() {
	register_editor(Editor::title, [] {
		return std::make_unique<Editor>();
	});
}

}
