#pragma once

#include "platform.hpp"
#include "scoped_context.hpp"
#include "vector4.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace nfwk {
class texture;
}

namespace nfwk::ui {

struct popup_item {

	std::u8string label;
	std::u8string shortcut;
	bool selected{ false };
	bool enabled{ true };
	std::function<void()> on_click;
	std::vector<popup_item> children;

	popup_item() = default;

	popup_item(const std::u8string& label, const std::u8string& shortcut = u8"", bool selected = false, bool enabled = true, std::function<void()> click = {}, std::vector<popup_item> children = {})
		: label{ label }, shortcut{ shortcut }, selected{ selected }, enabled{ enabled }, on_click{ std::move(click) }, children{ std::move(children) } {}

};

void separate();
void inline_next();
void new_line();
void text(std::u8string_view format, ...);
void colored_text(vector3f color, std::u8string_view format, ...);
void colored_text(vector4f color, std::u8string_view format, ...);
bool button(std::u8string_view label);
bool button(std::u8string_view label, vector2f size);
bool checkbox(std::u8string_view label, bool& value);
bool radio(std::u8string_view label, int& selected, int value);

template<typename T>
bool input(std::u8string_view label, T& value) {
	const char* label_chars = reinterpret_cast<const char*>(label.data());
	if constexpr (std::is_same<T, short>()) {
		return ImGui::InputScalar(label_chars, ImGuiDataType_S16, &value);
	} else if constexpr (std::is_same<T, unsigned short>()) {
		return ImGui::InputScalar(label_chars, ImGuiDataType_U16, &value);
	} else if constexpr (std::is_same<T, int>()) {
		return ImGui::InputScalar(label_chars, ImGuiDataType_S32, &value);
	} else if constexpr (std::is_same<T, unsigned int>()) {
		return ImGui::InputScalar(label_chars, ImGuiDataType_U32, &value);
	} else if constexpr (std::is_same<T, long long>()) {
		return ImGui::InputScalar(label_chars, ImGuiDataType_S64, &value);
	} else if constexpr (std::is_same<T, unsigned long long>()) {
		return ImGui::InputScalar(label_chars, ImGuiDataType_U64, &value);
	} else if constexpr (std::is_same<T, float>()) {
		return ImGui::InputScalar(label_chars, ImGuiDataType_Float, &value);
	} else if constexpr (std::is_same<T, double>()) {
		return ImGui::InputScalar(label_chars, ImGuiDataType_Double, &value);
	}
}

template<template<typename> typename Vector, typename T>
bool input(std::u8string_view label, Vector<T>& value) {
	const char* label_chars = reinterpret_cast<const char*>(label.data());
	if constexpr (std::is_same<T, char>()) {
		return ImGui::InputScalarN(label_chars, ImGuiDataType_S8, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, unsigned char>()) {
		return ImGui::InputScalarN(label_chars, ImGuiDataType_U8, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, short>()) {
		return ImGui::InputScalarN(label_chars, ImGuiDataType_S16, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, unsigned short>()) {
		return ImGui::InputScalarN(label_chars, ImGuiDataType_U16, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, int>()) {
		return ImGui::InputScalarN(label_chars, ImGuiDataType_S32, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, unsigned int>()) {
		return ImGui::InputScalarN(label_chars, ImGuiDataType_U32, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, long long>()) {
		return ImGui::InputScalarN(label_chars, ImGuiDataType_S64, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, unsigned long long>()) {
		return ImGui::InputScalarN(label_chars, ImGuiDataType_U64, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, float>()) {
		return ImGui::InputScalarN(label_chars, ImGuiDataType_Float, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, double>()) {
		return ImGui::InputScalarN(label_chars, ImGuiDataType_Double, &value.x, Vector<T>::components);
	}
}

bool input(std::u8string_view label, std::u8string& value);
bool input(std::u8string_view label, std::u8string& value, vector2f box_size);

void grid(vector2f offset, vector2f grid_size, vector4f color);
void rectangle(vector2f position, vector2f size, const vector4f& color);
void outline(vector2f position, vector2f size, const vector4f& color);
void image(const texture& texture_);
void image(const texture& texture_, vector2f size);
void image(const texture& texture_, vector2f size, vector2f uv0, vector2f uv1, vector4f tint, vector4f border);

std::optional<int> combo(std::u8string_view label, const std::vector<std::u8string>& values, int selected);
void popup(std::u8string_view id, std::vector<popup_item>& values);
std::optional<int> list(std::u8string_view label, const std::vector<std::u8string>& values, int selected, std::optional<int> view_count = std::nullopt);

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

scoped_logic window(std::u8string_view label, std::optional<vector2f> position = std::nullopt, std::optional<vector2f> size = std::nullopt, ImGuiWindowFlags flags = default_window_flags, bool* open = nullptr);
scoped_logic window(std::u8string_view label, ImGuiWindowFlags flags = 0, bool* open = nullptr);

bool is_hovered();
scoped_logic disable_if(bool disable);

scoped_logic menu(std::u8string_view label, bool enabled = true);
bool menu_item(std::u8string_view label);
bool menu_item(std::u8string_view label, std::u8string_view shortcut);
bool menu_item(std::u8string_view label, bool& checked, bool enabled = true);
bool menu_item(std::u8string_view label, std::u8string_view shortcut, bool& checked, bool enabled = true);

}
