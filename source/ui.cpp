#include "ui.hpp"
#include "color.hpp"
#include "assets.hpp"

#if ENABLE_GRAPHICS

namespace no::ui {

void separate() {
	ImGui::Separator();
}

void inline_next() {
	ImGui::SameLine();
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

bool checkbox(std::string_view label, bool& value) {
	return ImGui::Checkbox(label.data(), &value);
}

bool radio(std::string_view label, int& selected, int value) {
	return ImGui::RadioButton(label.data(), &selected, value);
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

bool input(std::string_view label, int& value) {
	return ImGui::InputInt(label.data(), &value);
}

bool input(std::string_view label, long long& value) {
	return ImGui::InputScalar(label.data(), ImGuiDataType_S64, &value);
}

bool input(std::string_view label, float& value) {
	return ImGui::InputFloat(label.data(), &value);
}

bool input(std::string_view label, double& value) {
	return ImGui::InputDouble(label.data(), &value);
}

bool input(std::string_view label, vector2i& value) {
	return ImGui::InputInt2(label.data(), &value.x);
}

bool input(std::string_view label, vector3i& value) {
	return ImGui::InputInt3(label.data(), &value.x);
}

bool input(std::string_view label, vector4i& value) {
	return ImGui::InputInt4(label.data(), &value.x);
}

bool input(std::string_view label, vector2l& value) {
	return ImGui::InputScalarN(label.data(), ImGuiDataType_S64, &value.x, 2);
}

bool input(std::string_view label, vector3l& value) {
	return ImGui::InputScalarN(label.data(), ImGuiDataType_S64, &value.x, 3);
}

bool input(std::string_view label, vector4l& value) {
	return ImGui::InputScalarN(label.data(), ImGuiDataType_S64, &value.x, 4);
}

bool input(std::string_view label, vector2f& value) {
	return ImGui::InputFloat2(label.data(), &value.x);
}

bool input(std::string_view label, vector3f& value) {
	return ImGui::InputFloat3(label.data(), &value.x);
}

bool input(std::string_view label, vector4f& value) {
	return ImGui::InputFloat4(label.data(), &value.x);
}

bool input(std::string_view label, vector2d& value) {
	return ImGui::InputScalarN(label.data(), ImGuiDataType_Double, &value.x, 2);
}

bool input(std::string_view label, vector3d& value) {
	return ImGui::InputScalarN(label.data(), ImGuiDataType_Double, &value.x, 3);
}

bool input(std::string_view label, vector4d& value) {
	return ImGui::InputScalarN(label.data(), ImGuiDataType_Double, &value.x, 4);
}

void grid(vector2f offset, vector2f grid_size, vector4f color) {
	auto draw_list = ImGui::GetWindowDrawList();
	draw_list->ChannelsSplit(2);
	const ImColor line_color{ color * 255.0f };
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
	ImGui::GetWindowDrawList()->AddRectFilled(position, position + size, ImColor{ color.x, color.y, color.z, color.w });
}

void outline(vector2f position, vector2f size, const vector4f& color) {
	ImGui::GetWindowDrawList()->AddRect(position, position + size, ImColor{ color.x, color.y, color.z, color.w });
}

std::optional<int> combo(std::string_view label, const std::vector<std::string>& values, int selected) {
	if (selected >= static_cast<int>(values.size())) {
		BUG("Index is too high.");
		return std::nullopt;
	}
	std::optional<int> clicked;
	if (ImGui::BeginCombo(label.data(), values[static_cast<size_t>(selected)].c_str())) {
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

static void popup_next_menu(const popup_item& item) {
	if (item.children.empty()) {
		if (ImGui::MenuItem(item.label.c_str(), item.shortcut.c_str(), item.selected, item.enabled)) {
			if (item.on_click) {
				item.on_click();
			}
		}
	} else {
		if (ImGui::BeginMenu(item.label.c_str(), item.enabled)) {
			for (const auto& child : item.children) {
				popup_next_menu(child);
			}
			ImGui::EndMenu();
		}
	}
}

void popup(std::string_view id, const std::vector<popup_item>& items) {
	if (!items.empty()) {
		if (ImGui::BeginPopup(id.data())) {
			for (const auto& item : items) {
				popup_next_menu(item);
			}
			ImGui::EndPopup();
		}
	}
}

std::optional<int> list(std::string_view label, const std::vector<std::string>& values, int selected) {
	std::optional<int> clicked;
	ImGui::ListBoxHeader(label.data(), values.size());
	for (int i{ 0 }; i < static_cast<int>(values.size()); i++) {
		if (ImGui::Selectable(values[i].c_str(), i == selected)) {
			clicked = i;
		}
	}
	ImGui::ListBoxFooter();
	return clicked;
}

void push_static_window(std::string_view label, vector2f position, vector2f size) {
	const ImGuiWindowFlags flags{
		  ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoTitleBar
	};
	ImGui::SetNextWindowPos(position, ImGuiCond_Once);
	ImGui::SetNextWindowSize(size, ImGuiCond_Always);
	ImGui::Begin(label.data(), nullptr, flags);
}

void push_window(std::string_view label, vector2f position, vector2f size) {
	const ImGuiWindowFlags flags{
		  ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoCollapse
	};
	ImGui::SetNextWindowPos(position, ImGuiCond_Once);
	ImGui::SetNextWindowSize(size, ImGuiCond_Always);
	ImGui::Begin(label.data(), nullptr, flags);
}

void pop_window() {
	ImGui::End();
}

bool is_hovered() {
	return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}

void begin_disabled() {
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
}

void end_disabled() {
	ImGui::PopStyleVar();
	ImGui::PopItemFlag();
}

}

#endif
