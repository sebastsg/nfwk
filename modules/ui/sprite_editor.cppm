module;

#include "assert.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

export module nfwk.ui:sprite_editor;

import std.core;
import std.filesystem;
import nfwk.core;
import nfwk.assets;
import :imgui_wrapper;
import :editor;

export namespace nfwk {

class sprite_animation_configuration {
public:

	std::string name;
	vector4i tex_coords;
	float fps{ 10.0f };
	std::uint16_t frames{ 1 };
	std::optional<std::uint8_t> loops;

	sprite_animation_configuration() {
		name = random_number_generator::global().string(10);
	}

	sprite_animation_configuration(io_stream& stream) {
		read(stream);
	}

	vector2f get_frame_size() const {
		return { static_cast<float>(tex_coords.z) / static_cast<float>(frames), static_cast<float>(tex_coords.w) };
	}

	void write(io_stream& stream) const {
		stream.write(name);
		stream.write(tex_coords);
		stream.write(fps);
		stream.write(frames);
		stream.write_optional<std::uint8_t>(loops);
	}

	void read(io_stream& stream) {
		name = stream.read<std::string>();
		tex_coords = stream.read<vector4i>();
		fps = stream.read<float>();
		frames = stream.read<std::uint16_t>();
		loops = stream.read_optional<std::uint8_t>();
	}

};

class sprite_configuration {
public:

	static constexpr std::string_view file_extension{ ".nfwk-sprite" };

	std::string name;
	std::string texture_name;
	std::vector<sprite_animation_configuration> animations;

	sprite_configuration() {
		name = random_number_generator::global().string(10);
	}

	sprite_configuration(io_stream& stream) {
		read(stream);
	}

	void load(const std::string& new_name) {
		name = new_name;
		if (io_stream stream{ get_path() }; !stream.empty()) {
			read(stream);
		} else {
			warning("ui", "Did not find {}", get_path());
		}
	}

	void save() const {
		ASSERT(!name.empty());
		io_stream stream;
		write(stream);
		write_file(get_path(), stream);
	}

	std::filesystem::path get_path() const {
		return asset_path("sprites/" + name + std::string{ file_extension });
	}

	void write(io_stream& stream) const {
		stream.write(name);
		stream.write(texture_name);
		stream.write(static_cast<std::int32_t>(animations.size()));
		for (const auto& animation : animations) {
			animation.write(stream);
		}
	}

	void read(io_stream& stream) {
		name = stream.read<std::string>();
		texture_name = stream.read<std::string>();
		const auto count = stream.read<std::int32_t>();
		for (int i{ 0 }; i < count; i++) {
			animations.emplace_back(stream);
		}
	}

};

}

namespace nfwk {
static std::vector<sprite_configuration> sprites;
}

export namespace nfwk {

void register_sprite(const sprite_configuration& sprite) {
	sprites.push_back(sprite);
}

const std::vector<sprite_configuration>& get_all_sprites() {
	return sprites;
}

class sprite_editor : public abstract_editor {
public:

	sprite_editor(const sprite_configuration& sprite) : sprite{ sprite }, saved_sprite{ sprite } {
		opened_sprites.push_back(this);
		//texture = require_texture(sprite.texture_name);
	}

	sprite_editor(const std::string& name) {
		sprite.load(name);
		saved_sprite = sprite;
		opened_sprites.push_back(this);
		//texture = require_texture(sprite.texture_name);
	}

	~sprite_editor() override {
		std::erase(opened_sprites, this);
	}

	bool is_dirty() const override {
		return dirty;
	}

	void update() override {
		if (should_focus) {
			ImGui::SetNextWindowFocus();
			should_focus = false;
		}
		const auto title = sprite.name + (is_dirty() ? "*" : "");
		// TODO: + this
		if (auto end = ui::window(title + "###" /*+ this*/, 0, &open)) {
			if (auto editor = static_cast<editor_state*>(program_state::current()); editor && !docked) {
				editor->dock(ImGuiDir_None, 0.0f);
				docked = true;
			}
			ImGui::GetWindowDrawList()->ChannelsSplit(2);
			ImGui::PushID(this);
			ImGui::PushItemWidth(320.0f);
			if (auto end_sprite = ui::section()) {
				dirty |= ui::input("Sprite Name", sprite.name);
				if (ui::input("Texture Name", sprite.texture_name)) {
					//texture = require_texture(sprite.texture_name);
					dirty = true;
				}
			}
			if (texture.has_value()) {
				if (auto end_image = ui::section(true)) {
					if (ui::button("Texture Preview", { static_cast<float>(texture_size(texture).x) + 16.0f, 24.0f })) {
						show_preview = !show_preview;
					}
					if (show_preview) {
						ui::image(texture);
					}
				}
			}
			ui::separate();
			ui::text("Animations");
			for (auto& animation : sprite.animations) {
				if (auto end_section = ui::section()) {
					ui::set_section_background({});
					ui::set_section_padding({ 16.0f, 0.0f });
					update_animation_editor(animation);
					ui::set_section_background({ 0.0f, 0.0f, 0.0f, 0.2f });
					ui::set_section_padding(16.0f);
				}
				ui::new_line();
			}
			ui::new_line();
			if (ui::button("Add animation")) {
				sprite.animations.emplace_back();
			}
			update_save();
			ImGui::PopItemWidth();
			ImGui::PopID();
			ImGui::GetWindowDrawList()->ChannelsMerge();
		}
	}

	const sprite_configuration& get_sprite() const {
		return sprite;
	}

	const sprite_configuration& get_saved_sprite() const {
		return saved_sprite;
	}

	static bool is_open(std::string_view name) {
		return std::any_of(opened_sprites.begin(), opened_sprites.end(), [name](const auto& editor) {
			return editor->get_saved_sprite().name == name;
		});
	}

	static bool is_dirty(std::string_view name) {
		for (const auto* editor : opened_sprites) {
			if (editor->get_saved_sprite().name == name) {
				return editor->is_dirty();
			}
		}
		return false;
	}

	static void focus(std::string_view name) {
		for (auto editor : opened_sprites) {
			if (editor->get_saved_sprite().name == name) {
				editor->should_focus = true;
				break;
			}
		}
	}

private:

	void update_animation_editor(sprite_animation_configuration& animation) {
		ImGui::PushID(&animation);

		vector2i sprite_size{ 1, 1 };
		if (texture.has_value()) {
			sprite_size = texture_size(texture);
		}
		if (texture.has_value()) {
			const auto size = sprite_size.to<float>();
			if (auto end = ui::section()) {
				auto& ui_animation = ui_animations[animation.name];
				ui_animation.frames = animation.frames;
				ui_animation.fps = animation.fps;
				ui_animation.set_tex_coords(animation.tex_coords.to<float>());
				ui_animation.update(1.0f / 60.0f);
				auto tex_coords = ui_animation.get_tex_coords();
				tex_coords.xy /= size;
				tex_coords.zw /= size;
				ui::text("Name:");
				ui::inline_next();
				ImGui::PushItemWidth(224.0f);
				dirty |= ui::input("##animation-name", animation.name);
				ImGui::PopItemWidth();
				ui::outlined_image(texture, animation.get_frame_size(), tex_coords, { 0.5f, 0.5f, 1.0f, 0.8f });
			}
		}

		if (auto end = ui::section(true)) {
			ImGui::PushItemWidth(96.0f);
			dirty |= ui::input("Frames Per Second", animation.fps);
			animation.fps = value_between(animation.fps, 0.0f, 999.0f);
			dirty |= ui::input("Total Frames", animation.frames);
			animation.frames = value_between(animation.frames, 1, 999);
			bool loop_forever{ !animation.loops.has_value() };
			if (ui::checkbox("Loop Forever", loop_forever)) {
				if (animation.loops.has_value()) {
					animation.loops = std::nullopt;
				} else {
					animation.loops = 1;
				}
				dirty = true;
			} else if (animation.loops.has_value()) {
				auto new_loops = static_cast<int>(animation.loops.value());
				ui::inline_next();
				dirty |= ui::input("Loops", new_loops);
				new_loops = value_between(new_loops, 1, 999);
				animation.loops = static_cast<std::uint8_t>(new_loops);
			}
			ImGui::PopItemWidth();
		}

		if (texture.has_value()) {
			const auto size = sprite_size.to<float>();
			if (auto end = ui::section(true)) {
				ImGui::PushItemWidth(224.0f);
				if (ui::input("Texture Coordinates", animation.tex_coords)) {
					animation.tex_coords.x = value_between(animation.tex_coords.x, 0, sprite_size.x - 1);
					animation.tex_coords.y = value_between(animation.tex_coords.y, 0, sprite_size.y - 1);
					animation.tex_coords.z = value_between(animation.tex_coords.z, 1, sprite_size.x - animation.tex_coords.x);
					animation.tex_coords.w = value_between(animation.tex_coords.w, 1, sprite_size.y - animation.tex_coords.y);
					dirty = true;
				}
				ImGui::PopItemWidth();

				auto tex_coords = animation.tex_coords.to<float>();
				tex_coords.xy /= size;
				tex_coords.zw /= size;
				ui::outlined_image(texture, animation.tex_coords.zw.to<float>(), tex_coords, { 0.5f, 0.5f, 1.0f, 0.8f });
			}
		}

		ImGui::PopID();
	}

	void update_save() {
		ui::separate();
		if (auto disabled = ui::disable_if(!is_dirty() || sprite.name.empty())) {
			if (ui::button("Save")) {
				sprite.save();
				saved_sprite = sprite;
				dirty = false;
			}
		}
	}

	bool should_focus{ false };
	bool docked{ false };
	bool dirty{ false };
	sprite_configuration sprite;
	sprite_configuration saved_sprite;
	scoped_context<int> texture;
	std::unordered_map<std::string, sprite_animation> ui_animations;
	bool show_preview{ false };

	static std::vector<sprite_editor*> opened_sprites;

};

class sprite_list_editor : public abstract_editor {
public:

	static constexpr std::string_view title{ "Sprites" };

	void update() override {
		// TODO: this
		if (auto end = ui::window(title /*+ "##" + this*/, 0, &open)) {
			if (auto editor = static_cast<editor_state*>(program_state::current()); editor && !docked) {
				editor->dock(ImGuiDir_Left, 0.2f);
				docked = true;
			}
			if (ui::button("Create new sprite")) {
				sprite_configuration sprite;
				register_sprite(sprite);
				if (auto editor = static_cast<editor_state*>(program_state::current())) {
					editor->open(std::make_unique<sprite_editor>(std::move(sprite)));
				}
			}
			ui::separate();

			const auto& sprites = get_all_sprites();
			const auto selected_index = ui::list("##sprite-list", [&sprites](int index) {
				const auto& name = sprites[index].name;
				return name + (sprite_editor::is_dirty(name) ? "*" : "");
			}, static_cast<int>(sprites.size()), -1);
			if (selected_index.has_value()) {
				const auto& sprite = sprites[selected_index.value()];
				if (sprite_editor::is_open(sprite.name)) {
					sprite_editor::focus(sprite.name);
				} else {
					if (auto editor = static_cast<editor_state*>(program_state::current())) {
						editor->open(std::make_unique<sprite_editor>(sprite));
					}
				}
			}
		}
	}

	bool is_dirty() const override {
		return false;
	}

private:

	bool docked{ false };

};

void initialize_sprites() {
	for (const auto& path : entries_in_directory(asset_path("sprites"), entry_inclusion::only_files, false)) {
		io_stream stream{ path };
		register_sprite({ stream });
	}
	register_editor<sprite_list_editor>();
}

}
