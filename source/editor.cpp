#include "editor.hpp"
#include "graphics/window.hpp"
#include "scripts/script_editor.hpp"
#include "presets.hpp"

namespace nfwk::ui {

static int get_imgui_docking_direction(docking_direction direction) {
	switch (direction) {
	case docking_direction::left: return ImGuiDir_Left;
	case docking_direction::right: return ImGuiDir_Right;
	case docking_direction::up: return ImGuiDir_Up;
	case docking_direction::down: return ImGuiDir_Down;
	case docking_direction::none: return ImGuiDir_None;
	}
}

window_container::window_container() {
	
}

window_container::~window_container() {
	
}

void window_container::update() {
	if (const auto* viewport = ImGui::GetMainViewport()) {
		vector2f position{ viewport->Pos.x, viewport->Pos.y };
		position.y += 24.0f;
		vector2f size{ viewport->Size.x, viewport->Size.y };
		ImGui::SetNextWindowPos(position);
		ImGui::SetNextWindowSize(size - position);
		ImGui::SetNextWindowViewport(viewport->ID);
	}
	constexpr ImGuiWindowFlags window_flags{
		//ImGuiWindowFlags_MenuBar |
		//ImGuiWindowFlags_NoMouseInputs |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus
	};
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
	ImGui::Begin("##container-root-window", nullptr, window_flags);
	ImGui::PopStyleVar(3);
	ImGui::DockSpace(ImGui::GetID("window-dock"), { 0.0f, 0.0f }, ImGuiDockNodeFlags_NoCloseButton);
	dock_node = ImGui::DockBuilderGetNode(ImGui::GetID("window-dock"));
	for (int i{ 0 }; i < static_cast<int>(windows.size()); i++) {
		auto& window = *windows[i];
		ImGui::PushID(&window);
		if (auto _ = window.begin_update()) {
			window.update(*this);
			if (window.needs_to_dock) {
				dock(window.direction, window.ratio);
				window.needs_to_dock = false;
			}
		}
		if (window.is_open()) {
			window.after_update();
		} else {
			close(window);
			i--;
		}
		ImGui::PopID();
	}
	ImGui::End();
	bool dirty{ false };
	for (const auto& editor : windows) {
		dirty |= editor->has_unsaved_changes();
	}
	//text("Saved: %s", dirty ? "No " : "Yes");
	//inline_next();
	//vector4f saved{ 0.2f, 1.0f, 0.4f, 0.7f };
	//vector4f unsaved{ 0.8f, 0.8f, 0.1f, 0.7f };
	//vector4f status{ dirty ? unsaved : saved };
	//vector4f border{ status + vector4f{ 0.2f, 0.2f, 0.2f, 0.0f } };
	//ui::image(*blank_texture, 10.0f, 0.0f, 1.0f, status, border);
}

void window_container::dock(docking_direction direction, float ratio) {
	const auto imgui_direction = get_imgui_docking_direction(direction);
	const auto node = imgui_direction == ImGuiDir_None ? dock_node->CentralNode : dock_node;
	auto window = ImGui::GetCurrentWindow();
	if (window->DockNode && window->DockNode->ID == node->ID) {
		warning(log, "Window is already docked: {}", window->Name);
	} else {
		ImGui::DockContextQueueDock(ImGui::GetCurrentContext(), dock_node->HostWindow, node, window, imgui_direction, ratio, false);
	}
}

void window_container::open(std::shared_ptr<window_wrapper> window) {
	window->open = true;
	windows.emplace_back(std::move(window));
}

void window_wrapper::dock(docking_direction new_direction, float new_ratio) {
	direction = new_direction;
	ratio = new_ratio;
	needs_to_dock = true;
}

scoped_logic window_wrapper::begin_update() {
	const auto id = get_unique_id();
	return window(get_name(), id.empty() ? std::to_string(reinterpret_cast<std::intptr_t>(this)) : id, get_flags(), &open);
}

void window_container::close(window_wrapper& window_to_close) {
	std::erase_if(windows, [&](const auto& open_editor) {
		return &window_to_close == open_editor.get();
	});
}

}
