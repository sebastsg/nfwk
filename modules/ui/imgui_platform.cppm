module;

#include <glew/glew.h>

#include <Windows.h>
#include <tchar.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_freetype.h>

export module nfwk.ui:imgui_platform;

import std.core;
import std.memory;
import nfwk.core;
import nfwk.graphics;
import nfwk.draw;
import :ui_style;

namespace nfwk::ui {

constexpr std::string_view vertex_glsl{
	"#version 330\n"
	"uniform mat4 model_view_projection;"
	"in vec2 in_Position;"
	"in vec2 in_TexCoords;"
	"in vec4 in_Color;"
	"out vec4 v_Color;"
	"out vec2 v_TexCoords;"
	"void main() {"
	"	gl_Position = model_view_projection * vec4(in_Position.xy, 0.0f, 1.0f);"
	"	v_Color = in_Color;"
	"	v_TexCoords = in_TexCoords;"
	"}"
};

constexpr std::string_view fragment_glsl{
	"#version 330\n"
	"uniform sampler2D active_texture;"
	"in vec4 v_Color;"
	"in vec2 v_TexCoords;"
	"out vec4 out_Color;"
	"void main() {"
	"	out_Color = texture(active_texture, v_TexCoords).rgba * v_Color;"
	"}"
};

struct imgui_vertex {
	static constexpr vertex_attribute_specification attributes[]{ 2, 2, { attribute_component::is_byte, 4, true } };
	vector2f position;
	vector2f tex_coords;
	std::uint32_t color{ 0 };
};

struct imgui_data {
	nfwk::window* window{ nullptr };
	long long time{ 0 };
	long long ticks_per_second{ 0 };
	system_cursor old_cursor{ system_cursor::none };
	int shader_id{ -1 };
	int font_texture_id{ -1 };
	event_listener keyboard_repeated_press;
	event_listener keyboard_release;
	event_listener keybord_input;
	event_listener mouse_scroll;
	event_listener mouse_cursor;
	event_listener mouse_press;
	event_listener mouse_double_click;
	event_listener mouse_release;
	ImFontConfig font_config{};
	vertex_array<imgui_vertex, unsigned short> vertex_array;

	ortho_camera camera;
};

std::unique_ptr<imgui_data> data;

bool is_initialized() {
	return data != nullptr;
}

void initialize_style() {
	auto& style = ImGui::GetStyle();
	style.Alpha = 1.0f;

	style.FrameRounding = 0.0f;
	style.ChildRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.WindowRounding = 0.0f;

	style.FramePadding = { 8.0f, 6.0f };

	const auto& ui_style = nfwk::ui::get_style();

	/*style.Colors[ImGuiCol_Text] = ui_style.window.normal.text;
	style.Colors[ImGuiCol_TextDisabled] = ui_style.window.disabled.text;
	style.Colors[ImGuiCol_WindowBg] = ui_style.window.normal.background;
	style.Colors[ImGuiCol_ChildBg] = ui_style.window.normal.background;
	style.Colors[ImGuiCol_PopupBg] = ui_style.window.normal.background;
	style.Colors[ImGuiCol_Border] = ui_style.window.normal.border;
	style.Colors[ImGuiCol_BorderShadow] = ui_style.window.normal.border;
	style.Colors[ImGuiCol_FrameBg] = ui_style.input.normal.background;
	style.Colors[ImGuiCol_FrameBgHovered] = ui_style.input.hover.background;
	style.Colors[ImGuiCol_FrameBgActive] = ui_style.input.active.background;
	style.Colors[ImGuiCol_TitleBg] = ui_style.window.normal.background;
	style.Colors[ImGuiCol_TitleBgCollapsed] = ui_style.window.normal.background;
	style.Colors[ImGuiCol_TitleBgActive] = ui_style.window.active.background;
	style.Colors[ImGuiCol_MenuBarBg] = ui_style.menu_item.disabled.background;
	style.Colors[ImGuiCol_ScrollbarBg] = ui_style.scrollbar.normal.background;
	style.Colors[ImGuiCol_ScrollbarGrab] = ui_style.scrollbar.normal.text;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ui_style.scrollbar.hover.text;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ui_style.scrollbar.active.text;
	style.Colors[ImGuiCol_CheckMark] = ui_style.button.normal.text;
	style.Colors[ImGuiCol_SliderGrab] = ui_style.button.normal.text;
	style.Colors[ImGuiCol_SliderGrabActive] = ui_style.button.active.text;
	style.Colors[ImGuiCol_Button] = ui_style.button.normal.background;
	style.Colors[ImGuiCol_ButtonHovered] = ui_style.button.hover.background;
	style.Colors[ImGuiCol_ButtonActive] = ui_style.button.active.background;
	style.Colors[ImGuiCol_Header] = ui_style.menu_item.normal.background;
	style.Colors[ImGuiCol_HeaderHovered] = ui_style.menu_item.hover.background;
	style.Colors[ImGuiCol_HeaderActive] = ui_style.menu_item.active.background;
	style.Colors[ImGuiCol_Separator] = ui_style.window.normal.border;
	style.Colors[ImGuiCol_SeparatorHovered] = ui_style.window.normal.border;
	style.Colors[ImGuiCol_SeparatorActive] = ui_style.window.normal.border;
	style.Colors[ImGuiCol_ResizeGrip] = ui_style.resize_grip.normal.background;
	style.Colors[ImGuiCol_ResizeGripHovered] = ui_style.resize_grip.hover.background;
	style.Colors[ImGuiCol_ResizeGripActive] = ui_style.resize_grip.active.background;
	style.Colors[ImGuiCol_Tab] = ui_style.button.normal.background;
	style.Colors[ImGuiCol_TabHovered] = ui_style.button.hover.background;
	style.Colors[ImGuiCol_TabActive] = ui_style.button.active.background;
	style.Colors[ImGuiCol_TabUnfocused] = ui_style.window.normal.background;
	style.Colors[ImGuiCol_TabUnfocusedActive] = ui_style.button.active.background;
	style.Colors[ImGuiCol_DockingEmptyBg] = ui_style.input.normal.background; // for contrast
	style.Colors[ImGuiCol_DockingPreview] = ui_style.button.hover.background;
	style.Colors[ImGuiCol_PlotLines] = ui_style.window.normal.text;
	style.Colors[ImGuiCol_PlotLinesHovered] = ui_style.window.normal.text;
	style.Colors[ImGuiCol_PlotHistogram] = ui_style.window.normal.text;
	style.Colors[ImGuiCol_PlotHistogramHovered] = ui_style.window.normal.text;
	style.Colors[ImGuiCol_TextSelectedBg] = ui_style.input.active.background;
	style.Colors[ImGuiCol_DragDropTarget] = ui_style.window.normal.text;
	style.Colors[ImGuiCol_NavHighlight] = ui_style.window.normal.text;
	style.Colors[ImGuiCol_NavWindowingHighlight] = ui_style.window.normal.text;
	style.Colors[ImGuiCol_NavWindowingDimBg] = ui_style.window.normal.text;
	style.Colors[ImGuiCol_ModalWindowDimBg] = ui_style.window.normal.text;*/
}

system_cursor imgui_to_system_cursor(ImGuiMouseCursor cursor) {
	switch (cursor) {
	case ImGuiMouseCursor_None: return system_cursor::none;
	case ImGuiMouseCursor_Arrow: return system_cursor::arrow;
	case ImGuiMouseCursor_TextInput: return system_cursor::beam;
	case ImGuiMouseCursor_ResizeAll: return system_cursor::resize_all;
	case ImGuiMouseCursor_ResizeEW: return system_cursor::resize_horizontal;
	case ImGuiMouseCursor_ResizeNS: return system_cursor::resize_vertical;
	case ImGuiMouseCursor_ResizeNESW: return system_cursor::resize_diagonal_from_bottom_left;
	case ImGuiMouseCursor_ResizeNWSE: return system_cursor::resize_diagonal_from_top_left;
	case ImGuiMouseCursor_Hand: return system_cursor::hand;
	default: return system_cursor::arrow;
	}
}

void update_cursor_icon() {
	auto& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) {
		return;
	}
	const auto new_cursor = imgui_to_system_cursor(io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor());
	if (data->old_cursor != new_cursor) {
		set_system_cursor(new_cursor);
		data->old_cursor = new_cursor;
	}
}

void update_mouse_position() {
	// todo: not windows specific
	auto& io = ImGui::GetIO();
	const auto window_handle = data->window->handle();
	if (io.WantSetMousePos) {
		POINT position{ static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y) };
		ClientToScreen(window_handle, &position);
		SetCursorPos(position.x, position.y);
	}
	io.MousePos = { -FLT_MAX, -FLT_MAX };
	if (POINT position; GetActiveWindow() == window_handle && GetCursorPos(&position) && ScreenToClient(window_handle, &position)) {
		io.MousePos = { static_cast<float>(position.x), static_cast<float>(position.y) };
	}
}

void set_mouse_down(mouse::button button, bool is_down) {
	auto& io = ImGui::GetIO();
	switch (button) {
	case mouse::button::left:
		io.MouseDown[0] = is_down;
		break;
	case mouse::button::right:
		io.MouseDown[1] = is_down;
		break;
	case mouse::button::middle:
		io.MouseDown[2] = is_down;
		break;
	}
}

}

export namespace nfwk::ui {

void create(nfwk::window& window, std::optional<std::string> font_name, int font_size) {
	if (is_initialized()) {
		warning("ui", "UI is already initialized.");
		return;
	}
	message("ui", "Initializing UI");
	ImGui::CreateContext();
	data = std::make_unique<imgui_data>();
	data->window = &window;
	data->ticks_per_second = performance_frequency();
	data->time = performance_counter();
	auto& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = false;
	io.BackendFlags = ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos;
	io.BackendPlatformName = "nfwk-windows";
	io.BackendRendererName = "nfwk-opengl";
	//io.ImeWindowHandle = window.platform_window()->handle();
	//auto& platform_io = ImGui::GetPlatformIO();
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.KeyMap[ImGuiKey_Backspace] = static_cast<int>(key::backspace);
	io.KeyMap[ImGuiKey_Tab] = static_cast<int>(key::tab);
	io.KeyMap[ImGuiKey_Enter] = static_cast<int>(key::enter);
	io.KeyMap[ImGuiKey_Escape] = static_cast<int>(key::escape);
	io.KeyMap[ImGuiKey_Space] = static_cast<int>(key::space);
	io.KeyMap[ImGuiKey_PageUp] = static_cast<int>(key::page_up);
	io.KeyMap[ImGuiKey_PageDown] = static_cast<int>(key::page_down);
	io.KeyMap[ImGuiKey_LeftArrow] = static_cast<int>(key::left);
	io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(key::right);
	io.KeyMap[ImGuiKey_UpArrow] = static_cast<int>(key::up);
	io.KeyMap[ImGuiKey_DownArrow] = static_cast<int>(key::down);
	io.KeyMap[ImGuiKey_End] = static_cast<int>(key::end);
	io.KeyMap[ImGuiKey_Home] = static_cast<int>(key::home);
	io.KeyMap[ImGuiKey_Insert] = static_cast<int>(key::insert);
	io.KeyMap[ImGuiKey_Delete] = static_cast<int>(key::del);
	io.KeyMap[ImGuiKey_A] = static_cast<int>(key::a);
	io.KeyMap[ImGuiKey_C] = static_cast<int>(key::c);
	io.KeyMap[ImGuiKey_V] = static_cast<int>(key::v);
	io.KeyMap[ImGuiKey_X] = static_cast<int>(key::x);
	io.KeyMap[ImGuiKey_Y] = static_cast<int>(key::y);
	io.KeyMap[ImGuiKey_Z] = static_cast<int>(key::z);
	data->keyboard_repeated_press = window.keyboard.repeated_press.listen([&](key pressed_key) {
		if (static_cast<int>(pressed_key) < 256) {
			io.KeysDown[static_cast<int>(pressed_key)] = true;
		}
	});
	data->keyboard_release = window.keyboard.release.listen([&](key released_key) {
		if (static_cast<int>(released_key) < 256) {
			io.KeysDown[static_cast<int>(released_key)] = false;
		}
	});
	data->keybord_input = window.keyboard.input.listen([&](unsigned int character) {
		if (character > 0 && character < 0x10000) {
			io.AddInputCharacter(character);
		}
	});
	data->mouse_scroll = window.mouse.scroll.listen([&](int steps) {
		io.MouseWheel += static_cast<float>(steps);
	});
	data->mouse_cursor = window.mouse.icon.listen([] {
		update_cursor_icon();
	});
	data->mouse_press = window.mouse.press.listen([&](mouse::button pressed_button) {
		if (!ImGui::IsAnyMouseDown() && !GetCapture()) {
			SetCapture(data->window->handle());
		}
		set_mouse_down(pressed_button, true);
	});
	data->mouse_double_click = window.mouse.double_click.listen([&](mouse::button pressed_button) {
		if (!ImGui::IsAnyMouseDown() && !GetCapture()) {
			SetCapture(data->window->handle());
		}
		set_mouse_down(pressed_button, true);
	});
	data->mouse_release = window.mouse.release.listen([&](mouse::button released_button) {
		set_mouse_down(released_button, false);
		if (!ImGui::IsAnyMouseDown() && GetCapture() == data->window->handle()) {
			ReleaseCapture();
		}
	});

	data->shader_id = create_shader_from_source(vertex_glsl, fragment_glsl);

	if (font_name.has_value()) {
		message("ui", "Using specified font for UI: {}", font_name.value());
		if (const auto font_path = font::find_absolute_path(font_name.value())) {
			static constexpr ImWchar glyph_ranges[]{
				1, 256,
				8592, 8592,
				0
			};
			//data.font_config.MergeMode = true;
			io.Fonts->AddFontFromFileTTF(font_path->c_str(), static_cast<float>(font_size), &data->font_config, glyph_ranges);
			ImGuiFreeType::BuildFontAtlas(io.Fonts);
		} else {
			warning("ui", "Did not find the font.");
		}
	} else {
		message("ui", "Using default font for UI.");
	}

	unsigned char* pixels{ nullptr };
	int width{ 0 };
	int height{ 0 };
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	data->font_texture_id = create_texture({ reinterpret_cast<std::uint32_t*>(pixels), width, height, pixel_format::rgba, surface::construct_by::copy });
	io.Fonts->TexID = reinterpret_cast<ImTextureID>(data->font_texture_id);
	initialize_style();
}

void destroy() {
	if (is_initialized()) {
		message("ui", "Destroying UI state");
		delete_shader(data->shader_id);
		delete_texture(data->font_texture_id);
		data.release();
		ImGui::GetIO().Fonts->TexID = 0;
	}
}

void start_frame() {
	if (is_initialized()) {
		auto& io = ImGui::GetIO();
		RECT rect;
		GetClientRect(data->window->handle(), &rect);
		io.DisplaySize = { static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top) };
		const long long current_time{ performance_counter() };
		io.DeltaTime = static_cast<float>(current_time - data->time) / static_cast<float>(data->ticks_per_second);
		data->time = current_time;
		io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
		io.KeySuper = false;
		update_mouse_position();
		ImGui::NewFrame();
	}
}

void end_frame() {
	if (is_initialized()) {
		update_cursor_icon();
		ImGui::Render();
	}
}

void draw() {
	if (!is_initialized()) {
		return;
	}
	auto draw_data = ImGui::GetDrawData();
	if (!draw_data) {
		warning("ui", "Invalid draw data.");
	}
	const auto& io = ImGui::GetIO();
	const vector2f display_position{ draw_data->DisplayPos.x, draw_data->DisplayPos.y };
	const vector2f display_size{ draw_data->DisplaySize.x, draw_data->DisplaySize.y };

	data->camera.transform.scale = display_size;

	bind_shader(data->shader_id);
	set_shader_view_projection(data->camera);

	vertex_array<imgui_vertex, unsigned short> vertex_array;
	for (int list_index{ 0 }; list_index < draw_data->CmdListsCount; list_index++) {
		const auto draw_list = draw_data->CmdLists[list_index];
		const auto vertex_data = reinterpret_cast<const std::uint8_t*>(draw_list->VtxBuffer.Data);
		const auto index_data = reinterpret_cast<const std::uint8_t*>(draw_list->IdxBuffer.Data);
		vertex_array.set(vertex_data, draw_list->VtxBuffer.Size, index_data, draw_list->IdxBuffer.Size);
		std::size_t offset{ 0 };
		for (int buffer_index{ 0 }; buffer_index < draw_list->CmdBuffer.Size; buffer_index++) {
			auto& buffer = draw_list->CmdBuffer[buffer_index];
			const auto current_offset = offset;
			offset += buffer.ElemCount;
			if (buffer.UserCallback) {
				if (buffer.UserCallback == ImDrawCallback_ResetRenderState) {
					warning("ui", "Not supported.");
				} else {
					buffer.UserCallback(draw_list, &buffer);
				}
			} else {
				const vector4f clip{
					buffer.ClipRect.x - display_position.x,
					buffer.ClipRect.y - display_position.y,
					buffer.ClipRect.z - display_position.x,
					buffer.ClipRect.w - display_position.y
				};
				if (display_size.x > clip.x && display_size.y > clip.y && clip.z >= 0.0f && clip.w >= 0.0f) {
					//const auto [sx, sy, sw, sh] = vector4f{ clip.x, display_size.y - clip.w, clip.z - clip.x, clip.w - clip.y }.to<int>();
					// todo: fix binding
					const auto& s = vector4f{ clip.x, display_size.y - clip.w, clip.z - clip.x, clip.w - clip.y }.to<int>();
					auto sx = s.x;
					auto sy = s.y;
					auto sw = s.z;
					auto sh = s.w;
					data->window->set_scissor(sx, sy, sw, sh);
					bind_texture(reinterpret_cast<int>(buffer.TextureId));
					vertex_array.draw(current_offset, buffer.ElemCount);
				}
			}
		}
	}
	data->window->reset_scissor();
}

}
