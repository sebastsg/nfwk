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

#include "GLM/gtc/matrix_transform.hpp"
#include "GLM/gtc/type_ptr.hpp"

#include "imgui.h"
#include "imgui_platform.h"

namespace no {

namespace ui {

static struct {
	window* window{ nullptr };
	INT64 time{ 0 };
	INT64 ticks_per_second{ 0 };
	ImGuiMouseCursor last_mouse_cursor{ ImGuiMouseCursor_COUNT };
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
} data;

struct imgui_vertex {
	static constexpr vertex_attribute_specification attributes[]{ 2, 2, { attribute_component::is_byte, 4, true } };
	vector2f position;
	vector2f tex_coords;
	uint32_t color{ 0 };
};

static void update_cursor_icon() {
	const auto& io{ ImGui::GetIO() };
	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) {
		return;
	}
	const auto cursor{ ImGui::GetMouseCursor() };
	if (cursor == ImGuiMouseCursor_None || io.MouseDrawCursor) {
		SetCursor(nullptr);
		return;
	}
	SetCursor(LoadCursor(nullptr, [cursor] {
		switch (cursor) {
		case ImGuiMouseCursor_Arrow: return IDC_ARROW;
		case ImGuiMouseCursor_TextInput: return IDC_IBEAM;
		case ImGuiMouseCursor_ResizeAll: return IDC_SIZEALL;
		case ImGuiMouseCursor_ResizeEW: return IDC_SIZEWE;
		case ImGuiMouseCursor_ResizeNS: return IDC_SIZENS;
		case ImGuiMouseCursor_ResizeNESW: return IDC_SIZENESW;
		case ImGuiMouseCursor_ResizeNWSE: return IDC_SIZENWSE;
		case ImGuiMouseCursor_Hand: return IDC_HAND;
		default: return IDC_ARROW;
		}
	}()));
}

static void update_mouse_position() {
	auto& io{ ImGui::GetIO() };
	if (io.WantSetMousePos) {
		POINT position{ static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y) };
		ClientToScreen(data.window->platform_window()->handle(), &position);
		SetCursorPos(position.x, position.y);
	}
	io.MousePos = { -FLT_MAX, -FLT_MAX };
	POINT position;
	if (GetActiveWindow() == data.window->platform_window()->handle() && GetCursorPos(&position)) {
		if (ScreenToClient(data.window->platform_window()->handle(), &position)) {
			io.MousePos = { static_cast<float>(position.x), static_cast<float>(position.y) };
		}
	}
}

static void set_mouse_down(mouse::button button, bool is_down) {
	auto& io{ ImGui::GetIO() };
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

void create(window& window) {
	ASSERT(!data.window);
	ImGui::CreateContext();
	data.window = &window;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&data.ticks_per_second));
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&data.time));
	auto& io{ ImGui::GetIO() };
	io.BackendFlags = ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos;
	io.BackendPlatformName = "win32";
	io.ImeWindowHandle = window.platform_window()->handle();
	io.KeyMap[ImGuiKey_Tab] = VK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Space] = VK_SPACE;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';
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
		io.MouseWheel += steps;
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
	io.BackendRendererName = "opengl-nfwk";

	ImGui::StyleColorsDark();

	data.shader_id = create_shader(asset_path("shaders/imgui"));

	unsigned char* pixels{ nullptr };
	int width{ 0 };
	int height{ 0 };
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	data.font_texture_id = create_texture();
	bind_texture(data.font_texture_id);
	load_texture(data.font_texture_id, { reinterpret_cast<uint32_t*>(pixels), width, height, pixel_format::rgba, surface::construct_by::copy });

	io.Fonts->TexID = reinterpret_cast<ImTextureID>(data.font_texture_id);
}

void destroy() {
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

void start_frame() {
	auto& io{ ImGui::GetIO() };
	RECT rect;
	GetClientRect(data.window->platform_window()->handle(), &rect);
	io.DisplaySize = { static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top) };
	INT64 current_time;
	QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
	io.DeltaTime = static_cast<float>(current_time - data.time) / static_cast<float>(data.ticks_per_second);
	data.time = current_time;
	io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	io.KeySuper = false;
	update_mouse_position();
	const ImGuiMouseCursor mouse_cursor{ io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor() };
	if (data.last_mouse_cursor != mouse_cursor) {
		data.last_mouse_cursor = mouse_cursor;
		update_cursor_icon();
	}
	ImGui::NewFrame();
}

void end_frame() {
	ImGui::Render();
}

void draw() {
	auto draw_data{ ImGui::GetDrawData() };
	if (!draw_data) {
		return;
	}
	const auto& io{ ImGui::GetIO() };
	const int fb_width{ static_cast<int>(draw_data->DisplaySize.x * io.DisplayFramebufferScale.x) };
	const int fb_height{ static_cast<int>(draw_data->DisplaySize.y * io.DisplayFramebufferScale.y) };
	if (fb_width <= 0 || fb_height <= 0) {
		return;
	}
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	const vector2f pos{ draw_data->DisplayPos.x, draw_data->DisplayPos.y };
	bind_shader(data.shader_id);

	ortho_camera camera;
	camera.transform.scale = { draw_data->DisplaySize.x, draw_data->DisplaySize.y };
	set_shader_view_projection(camera);

	vertex_array<imgui_vertex, unsigned short> vertex_array;
	for (int list_index{ 0 }; list_index < draw_data->CmdListsCount; list_index++) {
		const auto cmd_list{ draw_data->CmdLists[list_index] };
		const auto vertex_data{ reinterpret_cast<uint8_t*>(cmd_list->VtxBuffer.Data) };
		const auto index_data{ reinterpret_cast<uint8_t*>(cmd_list->IdxBuffer.Data) };
		vertex_array.set(vertex_data, cmd_list->VtxBuffer.Size, index_data, cmd_list->IdxBuffer.Size);
		size_t offset{ 0 };
		for (int buffer_index = 0; buffer_index < cmd_list->CmdBuffer.Size; buffer_index++) {
			auto buffer = &cmd_list->CmdBuffer[buffer_index];
			const size_t current_offset{ offset };
			offset += buffer->ElemCount;
			if (buffer->UserCallback) {
				buffer->UserCallback(cmd_list, buffer);
				continue;
			}
			const vector4i clip_rect{
				static_cast<int>(buffer->ClipRect.x - pos.x),
				static_cast<int>(buffer->ClipRect.y - pos.y),
				static_cast<int>(buffer->ClipRect.z - pos.x),
				static_cast<int>(buffer->ClipRect.w - pos.y)
			};
			if (clip_rect.x >= fb_width || clip_rect.y >= fb_height || clip_rect.z < 0 || clip_rect.w < 0) {
				continue;
			}
			bind_texture(reinterpret_cast<int>(buffer->TextureId));
			vertex_array.draw(current_offset, buffer->ElemCount);
		}
	}
}

}

}

#endif
