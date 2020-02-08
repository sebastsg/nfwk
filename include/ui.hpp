#pragma once

#include "platform.hpp"
#include "math.hpp"

#if ENABLE_GRAPHICS

#include <imgui/imgui.h>

#include <optional>
#include <functional>

namespace no::ui {

struct menu_item_result {
	std::string label;
	std::string shortcut;
	bool selected{ false };
	bool enabled{ true };
};

using get_popup_item = std::function<std::optional<menu_item_result>(int)>;

void separate();
void inline_next();
void text(std::string_view format, ...);
void colored_text(vector3f color, std::string_view format, ...);
void colored_text(vector4f color, std::string_view format, ...);
bool button(std::string_view label);
bool checkbox(std::string_view label, bool& value);
bool radio(std::string_view label, int& selected, int value);
bool input(std::string_view label, std::string& value);
bool input(std::string_view label, std::string& value, vector2f box_size);
bool input(std::string_view label, int& value);
bool input(std::string_view label, long long& value);
bool input(std::string_view label, float& value);
bool input(std::string_view label, double& value);
bool input(std::string_view label, vector2i& value);
bool input(std::string_view label, vector3i& value);
bool input(std::string_view label, vector4i& value);
bool input(std::string_view label, vector2l& value);
bool input(std::string_view label, vector3l& value);
bool input(std::string_view label, vector4l& value);
bool input(std::string_view label, vector2f& value);
bool input(std::string_view label, vector3f& value);
bool input(std::string_view label, vector4f& value);
bool input(std::string_view label, vector2d& value);
bool input(std::string_view label, vector3d& value);
bool input(std::string_view label, vector4d& value);
void grid(vector2f offset, vector2f grid_size, vector4f color);

template<typename T>
std::optional<T> combo(std::string_view label, const std::vector<std::string>& values, T selected) {
	if (selected >= static_cast<T>(values.size())) {
		return {};
	}
	std::optional<T> clicked;
	if (ImGui::BeginCombo(label.data(), values[static_cast<size_t>(selected)].c_str())) {
		for (int i{ 0 }; i < static_cast<int>(values.size()); i++) {
			if (ImGui::Selectable(values[i].c_str())) {
				clicked = static_cast<T>(i);
				break;
			}
		}
		ImGui::EndCombo();
	}
	return clicked;
}

std::optional<int> popup(std::string_view id, const get_popup_item& get_item);

void push_static_window(std::string_view label, vector2f position, vector2f size);
void pop_window();

}

#endif
