#include "platform.hpp"

#if ENABLE_IMGUI

#include <glew/glew.h>

#include <Windows.h>
#include <tchar.h>

#include "window.hpp"
#include "windows_window.hpp"
#include "assets.hpp"
#include "camera.hpp"
#include "surface.hpp"
#include "font.hpp"

#include "GLM/gtc/matrix_transform.hpp"
#include "GLM/gtc/type_ptr.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_platform.hpp"
#include "imgui/imgui_freetype.h"

namespace no::ui {

static struct {
	window* window{ nullptr };
	long long time{ 0 };
	long long ticks_per_second{ 0 };
	platform::system_cursor old_cursor{ platform::system_cursor::none };
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
} data;

struct imgui_vertex {
	static constexpr vertex_attribute_specification attributes[]{ 2, 2, { attribute_component::is_byte, 4, true } };
	vector2f position;
	vector2f tex_coords;
	uint32_t color{ 0 };
};

static bool is_initialized() {
	return data.window != nullptr;
}

static void initialize_style() {
	auto& style = ImGui::GetStyle();
	style.Alpha = 1.0f;
	style.FrameRounding = 0.0f;
	style.ChildRounding = 0.0f;
	style.GrabRounding = 0.0f;
	style.PopupRounding = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.WindowRounding = 0.0f;

	vector4f text{ 0.8f, 0.85f, 0.9f, 1.0f };
	vector4f button{ 0.2f, 0.25f, 0.3f, 1.0f };
	vector4f button_hover{ 0.3f, 0.6f, 1.0f, 1.0f };
	vector4f border{ 0.3f, 0.6f, 1.0f, 1.0f };
	vector4f background{ 0.1f, 0.1f, 0.1f, 1.0f };
	vector4f frame{ 0.2f, 0.2f, 0.2f, 1.0f };
	vector4f frame_hover{ 0.3f, 0.3f, 0.3f, 1.0f };
	vector4f unknown{ 1.0f, 0.0f, 0.0f, 1.0f };

	const auto with_alpha = [](const vector4f& color, float alpha) {
		return vector4f{ color.x, color.y, color.z, alpha };
	};
	
	style.Colors[ImGuiCol_Text] = text;
	style.Colors[ImGuiCol_TextDisabled] = with_alpha(text, 0.6f);
	style.Colors[ImGuiCol_WindowBg] = background;
	style.Colors[ImGuiCol_ChildBg] = frame;
	style.Colors[ImGuiCol_PopupBg] = background;
	style.Colors[ImGuiCol_Border] = border;
	style.Colors[ImGuiCol_BorderShadow] = with_alpha(border, 0.2f);
	style.Colors[ImGuiCol_FrameBg] = frame;
	style.Colors[ImGuiCol_FrameBgHovered] = frame_hover;
	style.Colors[ImGuiCol_FrameBgActive] = frame_hover;
	style.Colors[ImGuiCol_TitleBg] = background;
	style.Colors[ImGuiCol_TitleBgCollapsed] = background;
	style.Colors[ImGuiCol_TitleBgActive] = frame;
	style.Colors[ImGuiCol_MenuBarBg] = background;
	style.Colors[ImGuiCol_ScrollbarBg] = frame;
	style.Colors[ImGuiCol_ScrollbarGrab] = button;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = button_hover;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = background;
	style.Colors[ImGuiCol_CheckMark] = button;
	style.Colors[ImGuiCol_SliderGrab] = button;
	style.Colors[ImGuiCol_SliderGrabActive] = button;
	style.Colors[ImGuiCol_Button] = button;
	style.Colors[ImGuiCol_ButtonHovered] = button_hover;
	style.Colors[ImGuiCol_ButtonActive] = button_hover;
	style.Colors[ImGuiCol_Header] = button;
	style.Colors[ImGuiCol_HeaderHovered] = button_hover;
	style.Colors[ImGuiCol_HeaderActive] = unknown;
	style.Colors[ImGuiCol_Separator] = border;
	style.Colors[ImGuiCol_SeparatorHovered] = border;
	style.Colors[ImGuiCol_SeparatorActive] = border;
	style.Colors[ImGuiCol_ResizeGrip] = button;
	style.Colors[ImGuiCol_ResizeGripHovered] = button_hover;
	style.Colors[ImGuiCol_ResizeGripActive] = button_hover;
	style.Colors[ImGuiCol_Tab] = button;
	style.Colors[ImGuiCol_TabHovered] = button_hover;
	style.Colors[ImGuiCol_TabActive] = button_hover;
	style.Colors[ImGuiCol_TabUnfocused] = button_hover;
	style.Colors[ImGuiCol_TabUnfocusedActive] = button_hover;
	style.Colors[ImGuiCol_PlotLines] = unknown;
	style.Colors[ImGuiCol_PlotLinesHovered] = unknown;
	style.Colors[ImGuiCol_PlotHistogram] = unknown;
	style.Colors[ImGuiCol_PlotHistogramHovered] = unknown;
	style.Colors[ImGuiCol_TextSelectedBg] = button_hover;
	style.Colors[ImGuiCol_DragDropTarget] = frame;
	style.Colors[ImGuiCol_NavHighlight] = background;
	style.Colors[ImGuiCol_NavWindowingHighlight] = background;
	style.Colors[ImGuiCol_NavWindowingDimBg] = background;
	style.Colors[ImGuiCol_ModalWindowDimBg] = background;
}

static platform::system_cursor imgui_to_system_cursor(ImGuiMouseCursor cursor) {
	switch (cursor) {
	case ImGuiMouseCursor_None: return platform::system_cursor::none;
	case ImGuiMouseCursor_Arrow: return platform::system_cursor::arrow;
	case ImGuiMouseCursor_TextInput: return platform::system_cursor::beam;
	case ImGuiMouseCursor_ResizeAll: return platform::system_cursor::resize_all;
	case ImGuiMouseCursor_ResizeEW: return platform::system_cursor::resize_horizontal;
	case ImGuiMouseCursor_ResizeNS: return platform::system_cursor::resize_vertical;
	case ImGuiMouseCursor_ResizeNESW: return platform::system_cursor::resize_diagonal_from_bottom_left;
	case ImGuiMouseCursor_ResizeNWSE: return platform::system_cursor::resize_diagonal_from_top_left;
	case ImGuiMouseCursor_Hand: return platform::system_cursor::hand;
	default: return platform::system_cursor::arrow;
	}
}

static void update_cursor_icon() {
	auto& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) {
		return;
	}
	const auto new_cursor = imgui_to_system_cursor(io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor());
	if (data.old_cursor != new_cursor) {
		platform::set_system_cursor(new_cursor);
		data.old_cursor = new_cursor;
	}
}

static void update_mouse_position() {
	auto& io = ImGui::GetIO();
	const auto window_handle = data.window->platform_window()->handle();
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

static void set_mouse_down(mouse::button button, bool is_down) {
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
	default:
		break;
	}
}

void create(window& window, std::optional<std::string> font_name, int font_size) {
	if (is_initialized()) {
		BUG("UI is already initialized.");
		return;
	}
	ImGui::CreateContext();
	data.window = &window;
	data.ticks_per_second = platform::performance_frequency();
	data.time = platform::performance_counter();
	auto& io = ImGui::GetIO();
	io.BackendFlags = ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos;
	io.BackendPlatformName = "nfwk-windows";
	io.BackendRendererName = "nfwk-opengl";
	io.ImeWindowHandle = window.platform_window()->handle();
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.KeyMap[ImGuiKey_Backspace] = static_cast<int>(no::key::backspace);
	io.KeyMap[ImGuiKey_Tab] = static_cast<int>(no::key::tab);
	io.KeyMap[ImGuiKey_Enter] = static_cast<int>(no::key::enter);
	io.KeyMap[ImGuiKey_Escape] = static_cast<int>(no::key::escape);
	io.KeyMap[ImGuiKey_Space] = static_cast<int>(no::key::space);
	io.KeyMap[ImGuiKey_PageUp] = static_cast<int>(no::key::page_up);
	io.KeyMap[ImGuiKey_PageDown] = static_cast<int>(no::key::page_down);
	io.KeyMap[ImGuiKey_LeftArrow] = static_cast<int>(no::key::left);
	io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(no::key::right);
	io.KeyMap[ImGuiKey_UpArrow] = static_cast<int>(no::key::up);
	io.KeyMap[ImGuiKey_DownArrow] = static_cast<int>(no::key::down);
	io.KeyMap[ImGuiKey_End] = static_cast<int>(no::key::end);
	io.KeyMap[ImGuiKey_Home] = static_cast<int>(no::key::home);
	io.KeyMap[ImGuiKey_Insert] = static_cast<int>(no::key::insert);
	io.KeyMap[ImGuiKey_Delete] = static_cast<int>(no::key::del);
	io.KeyMap[ImGuiKey_A] = static_cast<int>(no::key::a);
	io.KeyMap[ImGuiKey_C] = static_cast<int>(no::key::c);
	io.KeyMap[ImGuiKey_V] = static_cast<int>(no::key::v);
	io.KeyMap[ImGuiKey_X] = static_cast<int>(no::key::x);
	io.KeyMap[ImGuiKey_Y] = static_cast<int>(no::key::y);
	io.KeyMap[ImGuiKey_Z] = static_cast<int>(no::key::z);
	data.keyboard_repeated_press = window.keyboard.repeated_press.listen([&](key pressed_key) {
		if (static_cast<int>(pressed_key) < 256) {
			io.KeysDown[static_cast<int>(pressed_key)] = true;
		}
	});
	data.keyboard_release = window.keyboard.release.listen([&](key released_key) {
		if (static_cast<int>(released_key) < 256) {
			io.KeysDown[static_cast<int>(released_key)] = false;
		}
	});
	data.keybord_input = window.keyboard.input.listen([&](unsigned int character) {
		if (character > 0 && character < 0x10000) {
			io.AddInputCharacter(character);
		}
	});
	data.mouse_scroll = window.mouse.scroll.listen([&](int steps) {
		io.MouseWheel += static_cast<float>(steps);
	});
	data.mouse_cursor = window.mouse.icon.listen([] {
		update_cursor_icon();
	});
	data.mouse_press = window.mouse.press.listen([&](mouse::button pressed_button) {
		if (!ImGui::IsAnyMouseDown() && !GetCapture()) {
			SetCapture(data.window->platform_window()->handle());
		}
		set_mouse_down(pressed_button, true);
	});
	data.mouse_double_click = window.mouse.double_click.listen([&](mouse::button pressed_button) {
		if (!ImGui::IsAnyMouseDown() && !GetCapture()) {
			SetCapture(data.window->platform_window()->handle());
		}
		set_mouse_down(pressed_button, true);
	});
	data.mouse_release = window.mouse.release.listen([&](mouse::button released_button) {
		set_mouse_down(released_button, false);
		if (!ImGui::IsAnyMouseDown() && GetCapture() == data.window->platform_window()->handle()) {
			ReleaseCapture();
		}
	});

	data.shader_id = create_shader(asset_path("shaders/imgui"));

	if (font_name.has_value()) {
		if (const auto font_path = font::find_absolute_path(font_name.value())) {
			static ImWchar glyph_ranges[]{
				1, 256,
				8592, 8592,
				0
			};
			//data.font_config.MergeMode = true;
			io.Fonts->AddFontFromFileTTF(font_path->c_str(), static_cast<float>(font_size), &data.font_config, glyph_ranges);
			ImGuiFreeType::BuildFontAtlas(io.Fonts);
		}
	}

	unsigned char* pixels{ nullptr };
	int width{ 0 };
	int height{ 0 };
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	data.font_texture_id = create_texture({ reinterpret_cast<uint32_t*>(pixels), width, height, pixel_format::rgba, surface::construct_by::copy });
	io.Fonts->TexID = reinterpret_cast<ImTextureID>(data.font_texture_id);
	initialize_style();
	//ImGui::StyleColorsDark();
}

void destroy() {
	if (is_initialized()) {
		data.keyboard_repeated_press.stop();
		data.keyboard_release.stop();
		data.keybord_input.stop();
		data.mouse_scroll.stop();
		data.mouse_cursor.stop();
		data.mouse_press.stop();
		data.mouse_double_click.stop();
		data.mouse_release.stop();
		delete_shader(data.shader_id);
		delete_texture(data.font_texture_id);
		data.shader_id = -1;
		data.font_texture_id = -1;
		ImGui::GetIO().Fonts->TexID = 0;
		data.window = nullptr;
	}
}

void start_frame() {
	if (is_initialized()) {
		auto& io = ImGui::GetIO();
		RECT rect;
		GetClientRect(data.window->platform_window()->handle(), &rect);
		io.DisplaySize = { static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top) };
		const long long current_time{ platform::performance_counter() };
		io.DeltaTime = static_cast<float>(current_time - data.time) / static_cast<float>(data.ticks_per_second);
		data.time = current_time;
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
		BUG("Invalid draw data.");
	}
	const auto& io = ImGui::GetIO();
	const vector2f display_position{ draw_data->DisplayPos };
	const vector2f display_size{ draw_data->DisplaySize };

	ortho_camera camera;
	camera.transform.scale = display_size;

	bind_shader(data.shader_id);
	set_shader_view_projection(camera);

	vertex_array<imgui_vertex, unsigned short> vertex_array;
	for (int list_index{ 0 }; list_index < draw_data->CmdListsCount; list_index++) {
		const auto cmd_list = draw_data->CmdLists[list_index];
		const auto vertex_data = reinterpret_cast<const uint8_t*>(cmd_list->VtxBuffer.Data);
		const auto index_data = reinterpret_cast<const uint8_t*>(cmd_list->IdxBuffer.Data);
		vertex_array.set(vertex_data, cmd_list->VtxBuffer.Size, index_data, cmd_list->IdxBuffer.Size);
		size_t offset{ 0 };
		for (int buffer_index{ 0 }; buffer_index < cmd_list->CmdBuffer.Size; buffer_index++) {
			auto& buffer = cmd_list->CmdBuffer[buffer_index];
			const auto current_offset = offset;
			offset += buffer.ElemCount;
			if (buffer.UserCallback) {
				if (buffer.UserCallback == ImDrawCallback_ResetRenderState) {
					BUG("Not supported.");
				} else {
					buffer.UserCallback(cmd_list, &buffer);
				}
			} else {
				const vector4f clip{
					buffer.ClipRect.x - display_position.x,
					buffer.ClipRect.y - display_position.y,
					buffer.ClipRect.z - display_position.x,
					buffer.ClipRect.w - display_position.y
				};
				if (display_size.x > clip.x && display_size.y > clip.y && clip.z >= 0.0f && clip.w >= 0.0f) {
					const auto [sx, sy, sw, sh] = vector4f{ clip.x, display_size.y - clip.w, clip.z - clip.x, clip.w - clip.y }.to<int>();
					data.window->scissor(sx, sy, sw, sh);
					bind_texture(reinterpret_cast<int>(buffer.TextureId));
					vertex_array.draw(current_offset, buffer.ElemCount);
				}
			}
		}
	}
	data.window->reset_scissor();
}

}

#endif
