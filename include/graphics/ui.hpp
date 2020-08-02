#pragma once

#include "platform.hpp"
#include "scoped_context.hpp"
#include "vector4.hpp"

#if ENABLE_GRAPHICS

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace no::ui {

struct popup_item {

	std::string label;
	std::string shortcut;
	bool selected{ false };
	bool enabled{ true };
	std::function<void()> on_click;
	std::vector<popup_item> children;

	popup_item() = default;

	popup_item(const std::string& label, const std::string& shortcut = "", bool selected = false, bool enabled = true, std::function<void()> click = {}, const std::vector<popup_item>& children = {})
		: label{ label }, shortcut{ shortcut }, selected{ selected }, enabled{ enabled }, on_click{ click }, children{ children }
	{
	}

};

void separate();
void inline_next();
void new_line();
void text(std::string_view format, ...);
void colored_text(vector3f color, std::string_view format, ...);
void colored_text(vector4f color, std::string_view format, ...);
bool button(std::string_view label);
bool button(std::string_view label, vector2f size);
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
void rectangle(vector2f position, vector2f size, const vector4f& color);
void outline(vector2f position, vector2f size, const vector4f& color);

std::optional<int> combo(std::string_view label, const std::vector<std::string>& values, int selected);
void popup(std::string_view id, std::vector<popup_item>& values);
std::optional<int> list(std::string_view label, const std::vector<std::string>& values, int selected, std::optional<int> view_count = std::nullopt);

constexpr int default_window_flags{
	ImGuiWindowFlags_NoResize |
	ImGuiWindowFlags_NoCollapse
};

constexpr int background_window_flags{
	default_window_flags |
	ImGuiWindowFlags_NoMove | 
	ImGuiWindowFlags_NoTitleBar |
	ImGuiWindowFlags_NoBringToFrontOnFocus
};

scoped_logic push_window(std::string_view label, std::optional<vector2f> position = std::nullopt, std::optional<vector2f> size = std::nullopt, ImGuiWindowFlags flags = default_window_flags, bool* open = nullptr);
void pop_window();

bool is_hovered();
void begin_disabled();
void end_disabled();

scoped_logic menu(std::string_view label, bool enabled = true);
bool menu_item(std::string_view label);
bool menu_item(std::string_view label, std::string_view shortcut);
bool menu_item(std::string_view label, bool& checked, bool enabled = true);
bool menu_item(std::string_view label, std::string_view shortcut, bool& checked, bool enabled = true);

}

#endif
