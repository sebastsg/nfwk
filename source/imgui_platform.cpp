#include "platform.hpp"

#include <glew/glew.h>

#include <Windows.h>
#include <tchar.h>

#include "assets.hpp"
#include "graphics/window.hpp"
#include "graphics/windows_window.hpp"
#include "graphics/ortho_camera.hpp"
#include "graphics/surface.hpp"
#include "graphics/font.hpp"
#include "graphics/vertex.hpp"
#include "graphics/vertex_array.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"

#include "GLM/gtc/matrix_transform.hpp"
#include "GLM/gtc/type_ptr.hpp"

#include "imgui/imgui.h"
#include "imgui_platform.hpp"
#include "imgui/imgui_freetype.h"

namespace nfwk::ui {

static constexpr std::u8string_view vertex_glsl{
	u8"#version 330\n"
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

static constexpr std::u8string_view fragment_glsl{
	u8"#version 330\n"
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
	window* window{ nullptr };
	long long time{ 0 };
	long long ticks_per_second{ 0 };
	platform::system_cursor old_cursor{ platform::system_cursor::none };
	std::unique_ptr<shader> ui_shader;
	std::unique_ptr<texture> font_texture;
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
};

static std::unique_ptr<imgui_data> data;

static bool is_initialized() {
	return data != nullptr;
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
	vector4f button{ 0.25f, 0.3f, 0.4f, 1.0f };
	vector4f button_hover{ 0.3f, 0.6f, 1.0f, 1.0f };
	vector4f button_active{ 0.2f, 0.5f, 0.95f, 1.0f };
	vector4f border{ 0.3f, 0.6f, 0.9f, 1.0f };
	vector4f background{ 0.12f, 0.16f, 0.24f, 1.0f };
	vector4f frame{ 0.1f, 0.1f, 0.1f, 1.0f };
	vector4f frame_hover{ 0.2f, 0.35f, 0.55f, 1.0f };
	vector4f checkmark{ 0.2f, 0.5f, 0.95f, 1.0f };
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
	style.Colors[ImGuiCol_CheckMark] = checkmark;
	style.Colors[ImGuiCol_SliderGrab] = button;
	style.Colors[ImGuiCol_SliderGrabActive] = button;
	style.Colors[ImGuiCol_Button] = button;
	style.Colors[ImGuiCol_ButtonHovered] = button_hover;
	style.Colors[ImGuiCol_ButtonActive] = button_hover;
	style.Colors[ImGuiCol_Header] = button;
	style.Colors[ImGuiCol_HeaderHovered] = button_hover;
	style.Colors[ImGuiCol_HeaderActive] = button_active;
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
	if (data->old_cursor != new_cursor) {
		platform::set_system_cursor(new_cursor);
		data->old_cursor = new_cursor;
	}
}

static void update_mouse_position() {
	auto& io = ImGui::GetIO();
	const auto* window = dynamic_cast<const platform::windows_window*>(data->window);
	const auto window_handle = window->handle();
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
	}
}

void add_font(const std::filesystem::path& name, int size) {
	message(log, u8"Adding UI font: {}", name);
	if (const auto path = font::find_absolute_path(name)) {
		static const ImWchar glyph_ranges[]{
			1, 256,
			8592, 8592,
			0
		};
		auto& io = ImGui::GetIO();
		//data.font_config.MergeMode = true;
		const auto& name_string = path->u8string();
		const auto* name_data = reinterpret_cast<const char*>(name_string.c_str());
		io.Fonts->AddFontFromFileTTF(name_data, static_cast<float>(size), &data->font_config, glyph_ranges);
	} else {
		warning(log, u8"Did not find the font.");
	}
}

void build_fonts() {
	auto& io = ImGui::GetIO();
	ImGuiFreeType::BuildFontAtlas(io.Fonts);
	unsigned char* pixels{ nullptr };
	int width{ 0 };
	int height{ 0 };
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	data->font_texture = std::make_unique<texture>(surface{ reinterpret_cast<std::uint32_t*>(pixels), width, height, pixel_format::rgba, surface::construct_by::copy });
	io.Fonts->TexID = reinterpret_cast<ImTextureID>(data->font_texture.get());
}

void create(window& window) {
	if (is_initialized()) {
		warning(log, u8"UI is already initialized.");
		ASSERT(false);
		return;
	}
	message(log, u8"Initializing UI");
	ImGui::CreateContext();
	data = std::make_unique<imgui_data>();
	data->window = &window;
	data->ticks_per_second = platform::performance_frequency();
	data->time = platform::performance_counter();
	auto& io = ImGui::GetIO();
	io.BackendFlags = ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos;
	io.BackendPlatformName = "nfwk-windows";
	io.BackendRendererName = "nfwk-opengl";
	//io.ImeWindowHandle = window.platform_window()->handle();
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
			const auto* window = dynamic_cast<const platform::windows_window*>(data->window);
			SetCapture(window->handle());
		}
		set_mouse_down(pressed_button, true);
	});
	data->mouse_double_click = window.mouse.double_click.listen([&](mouse::button pressed_button) {
		if (!ImGui::IsAnyMouseDown() && !GetCapture()) {
			const auto* window = dynamic_cast<const platform::windows_window*>(data->window);
			SetCapture(window->handle());
		}
		set_mouse_down(pressed_button, true);
	});
	data->mouse_release = window.mouse.release.listen([&](mouse::button released_button) {
		set_mouse_down(released_button, false);
		const auto* window = dynamic_cast<const platform::windows_window*>(data->window);
		if (!ImGui::IsAnyMouseDown() && GetCapture() == window->handle()) {
			ReleaseCapture();
		}
	});
	data->ui_shader = std::make_unique<shader>(vertex_glsl, fragment_glsl);
	initialize_style();
}

void destroy() {
	if (is_initialized()) {
		message(log, u8"Destroying UI state");
		data = nullptr;
		ImGui::GetIO().Fonts->TexID = 0;
	}
}

void start_frame() {
	if (is_initialized()) {
		auto& io = ImGui::GetIO();
		RECT rect;
		const auto* window = dynamic_cast<const platform::windows_window*>(data->window);
		GetClientRect(window->handle(), &rect);
		io.DisplaySize = { static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top) };
		const long long current_time{ platform::performance_counter() };
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

void draw(render_context& context) {
	if (!is_initialized()) {
		return;
	}
	auto draw_data = ImGui::GetDrawData();
	if (!draw_data) {
		warning(log, u8"Invalid draw data.");
		ASSERT(false);
	}
	const auto& io = ImGui::GetIO();
	const vector2f display_position{ draw_data->DisplayPos };
	const vector2f display_size{ draw_data->DisplaySize };

	ortho_camera camera;
	camera.transform.scale = display_size;

	data->ui_shader->set_view_projection(camera);

	auto window = dynamic_cast<platform::windows_window*>(data->window);

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
					warning(log, u8"Not supported.");
					ASSERT(false);
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
					const auto [sx, sy, sw, sh] = vector4f{ clip.x, display_size.y - clip.w, clip.z - clip.x, clip.w - clip.y }.to<int>();
					context.set_scissor(sx, sy, sw, sh);
					static_cast<texture*>(buffer.TextureId)->bind();
					vertex_array.draw(current_offset, buffer.ElemCount);
				}
			}
		}
	}
	const auto [width, height] = window->size();
	context.set_scissor(0, 0, width, height);
}

}
