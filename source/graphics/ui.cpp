#include "graphics/ui.hpp"
#include "graphics/color.hpp"
#include "graphics/texture.hpp"
#include "assets.hpp"

namespace nfwk::ui {

std::string make_id(std::string_view name, std::string_view id) {
	return std::format("{}###{}", name, id);
}

std::string make_id(const void* id) {
	return std::format("###{}", id);
}

void separate() {
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
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
	ImGui::PushStyleColor(ImGuiCol_Text, to_rgba(color));
	va_list args;
	va_start(args, format);
	ImGui::TextV(format.data(), args);
	va_end(args);
	ImGui::PopStyleColor();
}

void colored_text(vector4f color, std::string_view format, ...) {
	ImGui::PushStyleColor(ImGuiCol_Text, to_rgba(color));
	va_list args;
	va_start(args, format);
	ImGui::TextV(format.data(), args);
	va_end(args);
	ImGui::PopStyleColor();
}

bool button(std::string_view label) {
	return ImGui::Button(label.data());
}

bool button(std::string_view label, vector2f size) {
	return ImGui::Button(label.data(), size);
}

bool checkbox(std::string_view label, bool& value) {
	return ImGui::Checkbox(label.data(), &value);
}

bool radio(std::string_view label, int& selected, int value) {
	return ImGui::RadioButton(label.data(), &selected, value);
}

void push_button_color(const vector4f& normal, const vector4f& hovered) {
	ImGui::PushStyleColor(ImGuiCol_Button, normal);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);
}

void pop_button_color() {
	ImGui::PopStyleColor(2);
}

bool input(std::string_view label, std::string& value) {
	const int flags{ ImGuiInputTextFlags_CallbackResize };
	return ImGui::InputText(label.data(), value.data(), value.capacity(), flags, [](auto data) {
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
			auto string = static_cast<std::string*>(data->UserData);
			string->resize(data->BufTextLen);
			data->Buf = string->data();
		}
		return 0;
	}, &value);
}

bool input(std::string_view label, std::string& value, vector2f box_size) {
	const int flags{ ImGuiInputTextFlags_CallbackResize };
	return ImGui::InputTextMultiline(label.data(), value.data(), value.capacity(), box_size, flags, [](auto data) {
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
			auto string = static_cast<std::string*>(data->UserData);
			string->resize(data->BufTextLen);
			data->Buf = string->data();
		}
		return 0;
	}, &value);
}

void grid(vector2f offset, vector2f grid_size, vector4f color) {
	auto draw_list = ImGui::GetWindowDrawList();
	draw_list->ChannelsSplit(2);
	const ImColor line_color{ color };
	const vector2f win_pos{ ImGui::GetCursorScreenPos() };
	const vector2f canvas_size{ ImGui::GetWindowSize() };
	for (float x{ std::fmodf(offset.x, grid_size.x) }; x < canvas_size.x; x += grid_size.x) {
		draw_list->AddLine({ x + win_pos.x, win_pos.y }, { x + win_pos.x, canvas_size.y + win_pos.y }, line_color);
	}
	for (float y{ std::fmodf(offset.y, grid_size.y) }; y < canvas_size.y; y += grid_size.y) {
		draw_list->AddLine({ win_pos.x, y + win_pos.y }, { canvas_size.x + win_pos.x, y + win_pos.y }, line_color);
	}
	draw_list->ChannelsSetCurrent(0);
}

void rectangle(vector2f position, vector2f size, const vector4f& color) {
	ImGui::GetWindowDrawList()->AddRectFilled(position, position + size, ImColor{ color });
}

void outline(vector2f position, vector2f size, const vector4f& color) {
	ImGui::GetWindowDrawList()->AddRect(position, position + size, ImColor{ color });
}

void image(const texture& texture_) {
	ImGui::Image(reinterpret_cast<ImTextureID>(const_cast<texture*>(&texture_)), texture_.size().to<float>());
}

void image(const texture& texture_, vector2f size) {
	ImGui::Image(reinterpret_cast<ImTextureID>(const_cast<texture*>(&texture_)), size);
}

void image(const texture& texture_, vector2f size, vector2f uv0, vector2f uv1, vector4f tint, vector4f border) {
	ImGui::Image(reinterpret_cast<ImTextureID>(const_cast<texture*>(&texture_)), size, uv0, uv1, tint, border);
}

std::optional<int> combo(std::string_view label, const std::vector<std::string>& values, int selected) {
	if (selected >= static_cast<int>(values.size())) {
		error(log, "Index is too high. {} >= {}", selected, values.size());
		return std::nullopt;
	}
	std::optional<int> clicked;
	if (ImGui::BeginCombo(label.data(), values[static_cast<std::size_t>(selected)].c_str())) {
		for (int i{ 0 }; i < static_cast<int>(values.size()); i++) {
			if (ImGui::Selectable(values[i].c_str())) {
				clicked = i;
				break;
			}
		}
		ImGui::EndCombo();
	}
	return clicked;
}

static void popup_next_menu(popup_item& item) {
	if (item.children.empty()) {
		if (checkable_menu_item(item.label, item.shortcut, item.selected, item.enabled)) {
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

std::optional<int> list(std::string_view label, const std::vector<std::string>& values, int selected, std::optional<int> view_count) {
	std::optional<int> clicked;
	if (ImGui::ListBoxHeader(label.data(), values.size(), view_count.value_or(-1))) {
		for (int i{ 0 }; i < static_cast<int>(values.size()); i++) {
			if (ImGui::Selectable(values[i].c_str(), i == selected)) {
				clicked = i;
			}
		}
		ImGui::ListBoxFooter();
	}
	return clicked;
}

static scoped_logic make_window(std::string_view title, std::optional<vector2f> position, std::optional<vector2f> size, ImGuiWindowFlags flags, bool* open) {
	if (position.has_value()) {
		ImGui::SetNextWindowPos(position.value());
	}
	if (size.has_value()) {
		ImGui::SetNextWindowSize(size.value());
	}
	if (ImGui::Begin(title.data(), open, flags) && ((open && *open) || !open)) {
		return [] {
			ImGui::End();
		};
	} else {
		ImGui::End();
		return {};
	}
}

scoped_logic window(std::string_view title, std::optional<vector2f> position, std::optional<vector2f> size, ImGuiWindowFlags flags, bool* open) {
	return make_window(title, position, size, flags, open);
}

scoped_logic window(std::string_view title, ImGuiWindowFlags flags, bool* open) {
	return make_window(title, std::nullopt, std::nullopt, flags, open);
}

scoped_logic window(std::string_view title, std::string_view id, std::optional<vector2f> position, std::optional<vector2f> size, ImGuiWindowFlags flags, bool* open) {
	return make_window(make_id(title, id), position, size, flags, open);
}

scoped_logic window(std::string_view title, std::string_view id, ImGuiWindowFlags flags, bool* open) {
	return make_window(make_id(title, id), std::nullopt, std::nullopt, flags, open);
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
		return [] {};
	}
}

bool is_hovered() {
	return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}

scoped_logic menu(std::string_view label, bool enabled) {
	if (ImGui::BeginMenu(label.data(), enabled)) {
		return scoped_logic{ [] {
			ImGui::EndMenu();
		} };
	} else {
		return {};
	}
}

bool menu_item(std::string_view label, bool enabled) {
	return ImGui::MenuItem(label.data(), nullptr, false, enabled);
}

bool menu_item(std::string_view label, std::string_view shortcut, bool enabled) {
	return ImGui::MenuItem(label.data(), shortcut.data(), false, enabled);
}

bool checkable_menu_item(std::string_view label, bool& checked, bool enabled) {
	return ImGui::MenuItem(label.data(), nullptr, &checked, enabled);
}

bool checkable_menu_item(std::string_view label, std::string_view shortcut, bool& checked, bool enabled) {
	return ImGui::MenuItem(label.data(), shortcut.data(), &checked, enabled);
}

}
