module;

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

export module nfwk.ui:imgui_wrapper;

import std.core;
import nfwk.core;
import nfwk.random;
import nfwk.input;
import nfwk.graphics;
import nfwk.draw;
import :ui_style;

export namespace nfwk::ui {

class popup_item {
public:

	std::string label;
	std::string shortcut;
	bool selected{ false };
	bool enabled{ true };
	std::function<void()> on_click;
	std::vector<popup_item> children;

	popup_item() = default;

	popup_item(const std::string& label, const std::string& shortcut = "", bool selected = false, bool enabled = true, std::function<void()> click = {}, const std::vector<popup_item>& children = {})
		: label{ label }, shortcut{ shortcut }, selected{ selected }, enabled{ enabled }, on_click{ click }, children{ children } {}

};

}

namespace nfwk::ui {

float zoom{ 1.0f };

thread_local std::unordered_map<unsigned int, custom_style_state> custom_style_states;
thread_local std::vector<unsigned int> custom_style_stack;

thread_local vector4f section_background_color{ 0.0f, 0.0f, 0.0f, 0.2f };
thread_local vector2f section_margin;
thread_local vector2f section_padding{ 16.0f };
thread_local float tallest_section_height{ 0.0f };

ImVec2 im2(const vector2f& f) {
	return ImVec2{ f.x, f.y };
}
ImVec4 im4(const vector4f& f) {
	return ImVec4{ f.x, f.y, f.z, f.w };
}
vector2f v2(const ImVec2& f) {
	return vector2f{ f.x, f.y };
}
vector4f v4(const ImVec4& f) {
	return vector4f{ f.x, f.y, f.z, f.w };
}

void push_custom_style(std::string_view id, element_style& styles) {
	const auto full_id = ImGui::GetID(id.data());
	const auto& state = custom_style_states[full_id];
	custom_style_stack.push_back(full_id);
	const auto& style = styles.get(state.hovered, state.active, false);
	ImGui::PushStyleColor(ImGuiCol_Text, im4(style.text));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, im4(style.background));
	ImGui::PushStyleColor(ImGuiCol_Header, im4(style.background));
	ImGui::PushStyleColor(ImGuiCol_CheckMark, im4(style.text));
}

void pop_custom_style(bool active = false) {
	ImGui::PopStyleColor(4);
	auto& state = custom_style_states[custom_style_stack.back()];
	state.hovered = ImGui::IsItemHovered();
	state.active = active;
	custom_style_stack.pop_back();
}

}

export namespace nfwk::ui {

void separate() {
	ImGui::Separator();
}

void inline_next() {
	ImGui::SameLine();
}

void new_line() {
	ImGui::NewLine();
}

void text(std::string_view format, ...) {
	va_list args;
	va_start(args, format);
	ImGui::TextV(format.data(), args);
	va_end(args);
}

void colored_text(vector3f color, std::string_view format, ...) {
	ImGui::PushStyleColor(ImGuiCol_Text, im4(to_rgba(color)));
	va_list args;
	va_start(args, format);
	ImGui::TextV(format.data(), args);
	va_end(args);
	ImGui::PopStyleColor();
}

void colored_text(vector4f color, std::string_view format, ...) {
	ImGui::PushStyleColor(ImGuiCol_Text, im4(to_rgba(color)));
	va_list args;
	va_start(args, format);
	ImGui::TextV(format.data(), args);
	va_end(args);
	ImGui::PopStyleColor();
}

void image(int texture) {
	ImGui::Image(reinterpret_cast<ImTextureID>(texture), im2(texture_size(texture).to<float>()));
}

void image(int texture, vector2f size) {
	ImGui::Image(reinterpret_cast<ImTextureID>(texture), im2(size));
}

void image(int texture, vector2f size, vector4f tex_coords) {
	ImGui::Image(reinterpret_cast<ImTextureID>(texture), im2(size), im2(tex_coords.xy), im2(tex_coords.xy + tex_coords.zw));
}

void outlined_image(int texture, vector4f border_color) {
	ImGui::Image(reinterpret_cast<ImTextureID>(texture), im2(texture_size(texture).to<float>()), {}, {}, im4({ 1.0f }), im4(border_color));
}

void outlined_image(int texture, vector2f size, vector4f border_color) {
	ImGui::Image(reinterpret_cast<ImTextureID>(texture), im2(size), {}, {}, im4({ 1.0f }), im4(border_color));
}

void outlined_image(int texture, vector2f size, vector4f tex_coords, vector4f border_color) {
	ImGui::Image(reinterpret_cast<ImTextureID>(texture), im2(size), im2(tex_coords.xy), im2(tex_coords.xy + tex_coords.zw), im4({ 1.0f }), im4(border_color));
}

// todo: these setters are a bit ugly. should probably be factored.
void set_section_background(vector4f color) {
	section_background_color = color;
}

void set_section_margin(vector2f margin) {
	section_margin = margin;
}

void set_section_padding(vector2f padding) {
	section_padding = padding;
}

void rectangle(vector2f position, vector2f size, const vector4f& color) {
	ImGui::GetWindowDrawList()->AddRectFilled(im2(position), im2(position + size), ImColor{ im4(color) });
}

scoped_logic section(bool inlined = false, const std::function<void(vector2f, vector2f)>& on_end = {}) {
	auto margin = section_margin;
	if (inlined) {
		margin += { section_padding.x, -section_padding.y };
		ui::inline_next();
	} else {
		tallest_section_height = 0.0f;
	}
	const auto top_left = margin + v2(ImGui::GetCursorScreenPos());
	ImGui::SetCursorScreenPos(im2(top_left + section_padding));
	ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);
	ImGui::BeginGroup();
	return [=] {
		ImGui::EndGroup();
		auto group_size = section_padding * 2.0f + v2(ImGui::GetItemRectSize());
		tallest_section_height = std::max(tallest_section_height, group_size.y);
		group_size.y = tallest_section_height;
		ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);
		ui::rectangle(top_left - 2.0f, group_size + 4.0f, section_background_color * 0.25f);
		ui::rectangle(top_left, group_size, section_background_color);
		if (on_end) {
			on_end(top_left, group_size);
		}
		ImGui::SetCursorScreenPos(im2(top_left + vector2f{ 0.0f, group_size.y }));
	};
}

bool button(std::string_view label) {
	push_custom_style(label, get_style().button);
	const bool pressed = ImGui::Button(label.data());
	pop_custom_style();
	return pressed;
}

bool button(std::string_view label, vector2f size) {
	push_custom_style(label, get_style().button);
	const bool pressed = ImGui::Button(label.data(), im2(size * zoom));
	pop_custom_style();
	return pressed;
}

bool checkbox(std::string_view label, bool& value) {
	push_custom_style(label, get_style().menu_item);
	const bool pressed = ImGui::Checkbox(label.data(), &value);
	pop_custom_style();
	return pressed;
}

bool radio(std::string_view label, int& selected, int value) {
	push_custom_style(label, get_style().menu_item);
	const bool pressed = ImGui::RadioButton(label.data(), &selected, value);
	pop_custom_style();
	return pressed;
}

bool input(std::string_view label, std::string& value) {
	push_custom_style(label, get_style().input);
	const int flags{ ImGuiInputTextFlags_CallbackResize };
	const bool changed = ImGui::InputText(label.data(), value.data(), value.capacity(), flags, [](auto data) {
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
			auto string = static_cast<std::string*>(data->UserData);
			string->resize(data->BufTextLen);
			data->Buf = string->data();
		}
		return 0;
	}, &value);
	pop_custom_style();
	return changed;
}

bool input(std::string_view label, std::string& value, vector2f box_size) {
	push_custom_style(label, get_style().input);
	box_size *= zoom;
	const int flags{ ImGuiInputTextFlags_CallbackResize };
	const bool changed = ImGui::InputTextMultiline(label.data(), value.data(), value.capacity(), im2(box_size), flags, [](auto data) {
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
			auto string = static_cast<std::string*>(data->UserData);
			string->resize(data->BufTextLen);
			data->Buf = string->data();
		}
		return 0;
	}, &value);
	pop_custom_style();
	return changed;
}

template<typename T>
bool input(std::string_view label, T& value) {
	if constexpr (std::is_same<T, short>()) {
		return ImGui::InputScalar(label.data(), ImGuiDataType_S16, &value);
	} else if constexpr (std::is_same<T, unsigned short>()) {
		return ImGui::InputScalar(label.data(), ImGuiDataType_U16, &value);
	} else if constexpr (std::is_same<T, int>()) {
		return ImGui::InputScalar(label.data(), ImGuiDataType_S32, &value);
	} else if constexpr (std::is_same<T, unsigned int>()) {
		return ImGui::InputScalar(label.data(), ImGuiDataType_U32, &value);
	} else if constexpr (std::is_same<T, long long>()) {
		return ImGui::InputScalar(label.data(), ImGuiDataType_S64, &value);
	} else if constexpr (std::is_same<T, unsigned long long>()) {
		return ImGui::InputScalar(label.data(), ImGuiDataType_U64, &value);
	} else if constexpr (std::is_same<T, float>()) {
		return ImGui::InputScalar(label.data(), ImGuiDataType_Float, &value);
	} else if constexpr (std::is_same<T, double>()) {
		return ImGui::InputScalar(label.data(), ImGuiDataType_Double, &value);
	}
}

template<template<typename> typename Vector, typename T>
bool input(std::string_view label, Vector<T>& value) {
	if constexpr (std::is_same<T, char>()) {
		return ImGui::InputScalarN(label.data(), ImGuiDataType_S8, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, unsigned char>()) {
		return ImGui::InputScalarN(label.data(), ImGuiDataType_U8, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, short>()) {
		return ImGui::InputScalarN(label.data(), ImGuiDataType_S16, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, unsigned short>()) {
		return ImGui::InputScalarN(label.data(), ImGuiDataType_U16, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, int>()) {
		return ImGui::InputScalarN(label.data(), ImGuiDataType_S32, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, unsigned int>()) {
		return ImGui::InputScalarN(label.data(), ImGuiDataType_U32, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, long long>()) {
		return ImGui::InputScalarN(label.data(), ImGuiDataType_S64, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, unsigned long long>()) {
		return ImGui::InputScalarN(label.data(), ImGuiDataType_U64, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, float>()) {
		return ImGui::InputScalarN(label.data(), ImGuiDataType_Float, &value.x, Vector<T>::components);
	} else if constexpr (std::is_same<T, double>()) {
		return ImGui::InputScalarN(label.data(), ImGuiDataType_Double, &value.x, Vector<T>::components);
	}
}

void grid(vector2f offset, vector2f grid_size, vector4f color) {
	auto draw_list = ImGui::GetWindowDrawList();
	draw_list->ChannelsSplit(2);
	const ImColor line_color{ im4(color) };
	const vector2f position{ v2(ImGui::GetCursorScreenPos()) };
	const vector2f size{ v2(ImGui::GetWindowSize()) };
	for (float x{ std::fmodf(offset.x, grid_size.x) }; x < size.x; x += grid_size.x) {
		draw_list->AddLine({ x + position.x, position.y }, { x + position.x, size.y + position.y }, line_color);
	}
	for (float y{ std::fmodf(offset.y, grid_size.y) }; y < size.y; y += grid_size.y) {
		draw_list->AddLine({ position.x, y + position.y }, { size.x + position.x, y + position.y }, line_color);
	}
	draw_list->ChannelsSetCurrent(0);
}

void outline(vector2f position, vector2f size, const vector4f& color) {
	ImGui::GetWindowDrawList()->AddRect(im2(position), im2(position + size), ImColor{ im4(color) });
}

std::optional<int> combo(std::string_view label, const std::vector<std::string>& values, int selected) {
	if (selected >= static_cast<int>(values.size())) {
		warning("ui", "Index is too high.");
		return std::nullopt;
	}
	std::optional<int> clicked;
	push_custom_style(label, get_style().menu_item);
	if (ImGui::BeginCombo(label.data(), values[static_cast<size_t>(selected)].c_str())) {
		for (int i{ 0 }; i < static_cast<int>(values.size()); i++) {
			push_custom_style(values[i].c_str(), get_style().menu_item);
			if (ImGui::Selectable(values[i].c_str(), i == selected)) {
				if (i != selected) {
					clicked = i;
				}
				pop_custom_style();
				break;
			} else {
				pop_custom_style();
			}
		}
		ImGui::EndCombo();
		pop_custom_style(true);
	} else {
		pop_custom_style();
	}
	return clicked;
}

scoped_logic menu(std::string_view label, bool enabled = true) {
	push_custom_style(label, get_style().menu_item);
	if (ImGui::BeginMenu(label.data(), enabled)) {
		return [] {
			ImGui::EndMenu();
			pop_custom_style(true);
		};
	} else {
		pop_custom_style();
		return {};
	}
}

bool menu_item(std::string_view label, std::string_view shortcut, bool& checked, bool enabled = true) {
	push_custom_style(label, get_style().menu_item);
	const bool pressed = ImGui::MenuItem(label.data(), shortcut.data(), &checked, enabled);
	pop_custom_style();
	return pressed;
}

bool menu_item(std::string_view label, bool& checked, bool enabled = true) {
	return menu_item(label, "", checked, enabled);
}

bool menu_item(std::string_view label, std::string_view shortcut) {
	bool checked{ false };
	return menu_item(label, shortcut, checked, true);
}

bool menu_item(std::string_view label) {
	return menu_item(label, "");
}

}

namespace nfwk::ui {

void popup_next_menu(popup_item& item) {
	if (item.children.empty()) {
		if (menu_item(item.label, item.shortcut, item.selected, item.enabled)) {
			if (item.on_click) {
				item.on_click();
			}
		}
	} else if (auto end = menu(item.label, item.enabled)) {
		for (auto& child : item.children) {
			popup_next_menu(child);
		}
	}
}

}

export namespace nfwk::ui {

void popup(std::string_view id, std::vector<popup_item>& items) {
	if (!items.empty()) {
		if (ImGui::BeginPopup(id.data())) {
			for (auto& item : items) {
				popup_next_menu(item);
			}
			ImGui::EndPopup();
		}
	}
}

std::optional<int> list(std::string_view label, const std::vector<std::string>& values, int selected, std::optional<int> view_count = std::nullopt) {
	std::optional<int> clicked;
	push_custom_style(label, get_style().input);
	if (ImGui::ListBoxHeader(label.data(), values.size(), view_count.value_or(-1))) {
		for (int i{ 0 }; i < static_cast<int>(values.size()); i++) {
			push_custom_style(values[i].c_str(), get_style().menu_item);
			if (ImGui::Selectable(values[i].c_str(), i == selected)) {
				clicked = i;
			}
			pop_custom_style();
		}
		ImGui::ListBoxFooter();
		pop_custom_style(true);
	} else {
		pop_custom_style();
	}
	return clicked;
}

std::optional<int> list(std::string_view label, const std::function<std::string(int)>& get_value, int count, int selected, std::optional<int> view_count = std::nullopt) {
	std::optional<int> clicked;
	push_custom_style(label, get_style().input);
	if (ImGui::ListBoxHeader(label.data(), count, view_count.value_or(-1))) {
		for (int i{ 0 }; i < static_cast<int>(count); i++) {
			push_custom_style(get_value(i).c_str(), get_style().menu_item);
			if (ImGui::Selectable(get_value(i).c_str(), i == selected)) {
				clicked = i;
			}
			pop_custom_style();
		}
		ImGui::ListBoxFooter();
		pop_custom_style(true);
	} else {
		pop_custom_style();
	}
	return clicked;
}

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

scoped_logic window(std::string_view label, ImGuiWindowFlags flags = 0, bool* open = nullptr) {
	if (ImGui::Begin(label.data(), open, flags)) {
		return [] {
			ImGui::End();
		};
	} else {
		ImGui::End();
		return {};
	}
}
scoped_logic window(std::string_view label, std::optional<vector2f> position, std::optional<vector2f> size, ImGuiWindowFlags flags = 0, bool* open = nullptr) {
	if (position.has_value()) {
		ImGui::SetNextWindowPos(im2(position.value()));
	}
	if (size.has_value()) {
		ImGui::SetNextWindowSize(im2(size.value()));
	}
	return window(label, flags, open);
}

bool is_hovered() {
	return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}

scoped_logic disable_if(bool disable) {
	if (disable) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		return [] {
			ImGui::PopStyleVar();
			ImGui::PopItemFlag();
		};
	} else {
		return {};
	}
}

void set_zoom(float new_zoom) {
	zoom = new_zoom;
}

template<typename T>
class search_input {
public:

	using search_function = std::function<std::vector<T>(std::string, int)>;

	search_input() : id{ random_number_generator::global().string(20) } {}
	search_input(key submit_key) : search_input{}, submit_key{ submit_key } {}
	search_input(const search_input&) = delete;
	search_input(search_input&&) = delete;

	virtual ~search_input() = default;

	search_input& operator=(const search_input&) = delete;
	search_input& operator=(search_input&&) = delete;

	[[nodiscard]] virtual std::optional<T> update(const search_function& search) {
		ImGui::PushID(id.c_str());
		if (nfwk::ui::input("##search-term", search_term)) {
			results = search(search_term, 10);
			last_selected_index = -1;
		}
		std::optional<T> selected;
		if (ImGui::IsKeyPressed(static_cast<int>(submit_key)) && !results.empty()) {
			selected = results[0];
			reset();
		}
		if (!results.empty()) {
			if (ImGui::ListBoxHeader("##result-list", static_cast<int>(results.size()))) {
				int index{ 0 };
				for (const auto& result : results) {
					// todo: should probably make this more general. this is fine for most cases anyway.
					std::string text{ "--" };
					if constexpr (std::is_same_v<std::string, T>) {
						text = result;
					} else {
						if constexpr (is_pointer<T>::value) {
							text = result->to_string();
						} else {
							text = result.to_string();
						}
					}
					if (ImGui::Selectable(text.c_str(), last_selected_index == index)) {
						selected = result;
					}
					index++;
				}
				ImGui::ListBoxFooter();
			}
		}
		ImGui::PopID();
		return selected;
	}

	void reset() {
		search_term = "";
		last_selected_index = -1;
		results.clear();
	}

	const std::vector<T>& get_results() const {
		return results;
	}

	std::string_view get_search_term() const {
		return search_term;
	}

protected:

	std::string id;
	std::string search_term;
	std::vector<T> results;
	int last_selected_index{ 0 };
	key submit_key{ key::enter };

};

#if 0
template<typename T>
class search_popup : public search_input<T> {
public:

	search_popup() = default;
	search_popup(key submit_key) : search_input<T>{ submit_key } {}

	[[nodiscard]] std::optional<T> update(const search_function& search) override {
		std::optional<T> selected;
		if (ImGui::IsPopupOpen(id.c_str())) {
			if (ImGui::BeginPopup(id.c_str())) {
				if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)) {
					ImGui::SetKeyboardFocusHere();
				}
				selected = search_input<T>::update(search);
				ImGui::EndPopup();
			}
		}
		return selected;
	}

	void open() {
		ImGui::OpenPopup(id.c_str());
	}

};
#endif

}
