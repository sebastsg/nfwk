#pragma once

#include <string>
#include <functional>

namespace no {

class font;
class audio_source;

using load_asset_func = std::function<void*()>;
using free_asset_func = std::function<void(void*)>;

namespace internal {

void* require_asset(const std::string& name);

}

void set_asset_directory(const std::string& path);
std::string asset_directory();
std::string asset_path(const std::string& path);

void register_asset(const std::string& name, const load_asset_func& load, const free_asset_func& free);

template<typename T>
T require_asset(const std::string& name) {
	return reinterpret_cast<T>(internal::require_asset(name));
}

void release_asset(const std::string& name);
void free_released_assets();

void register_texture(const std::string& name);
void register_all_textures();
int require_texture(const std::string& name);
void release_texture(const std::string& name);

void register_font(const std::string& name, int size);
font* require_font(const std::string& name, int size);
void release_font(const std::string& name, int size);

void register_shader(const std::string& name);
int require_shader(const std::string& name);
void release_shader(const std::string& name);

void register_sound(const std::string& name);
audio_source* require_sound(const std::string& name);
void release_sound(const std::string& name);


}
