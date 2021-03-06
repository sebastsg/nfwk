#pragma once

#include "scoped_context.hpp"

#include <filesystem>

namespace no {

class font;
class audio_source;

using load_asset_function = std::function<void*()>;
using free_asset_function = std::function<void(void*)>;

namespace internal {

void* require_asset(const std::string& name);

}

void set_asset_directory(const std::filesystem::path& path);
std::filesystem::path asset_directory();
std::filesystem::path asset_path(const std::string& path);

void register_asset(const std::string& name, const load_asset_function& load, const free_asset_function& free);

template<typename T>
T require_asset(const std::string& name) {
	return reinterpret_cast<T>(internal::require_asset(name));
}

void release_asset(const std::string& name);
void free_released_assets();

void register_texture(const std::string& name);
void register_all_textures();
scoped_context<int> require_texture(const std::string& name);

void register_font(const std::string& name, int size);
scoped_context<font*> require_font(const std::string& name, int size);

void register_shader(const std::string& name);
scoped_context<int> require_shader(const std::string& name);

void register_sound(const std::string& name);
scoped_context<audio_source*> require_sound(const std::string& name);


}
