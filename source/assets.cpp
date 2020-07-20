#include "assets.hpp"
#include "debug.hpp"
#include "draw.hpp"
#include "surface.hpp"
#include "font.hpp"
#include "ogg_vorbis.hpp"

#include <unordered_map>
#include <unordered_set>
#include <filesystem>

namespace no {

struct registered_asset {
	load_asset_function load;
	free_asset_function free;
	int holders{ 0 };
	void* any{ nullptr };
	bool is_loaded{ false };
};

static std::filesystem::path asset_directory_path{ "." };
static std::unordered_map<std::string, registered_asset> assets;
static std::unordered_set<std::string> to_be_released;

namespace internal {

void* require_asset(const std::string& name) {
	if (auto asset = assets.find(name); asset != assets.end()) {
		asset->second.holders++;
		if (asset->second.is_loaded) {
			to_be_released.erase(name);
		} else {
			asset->second.any = asset->second.load();
			asset->second.is_loaded = true;
		}
		return asset->second.any;
	} else {
		return nullptr;
	}
}

}

void set_asset_directory(const std::filesystem::path& path) {
	asset_directory_path = path;
}

std::filesystem::path asset_directory() {
	return asset_directory_path;
}

std::filesystem::path asset_path(const std::string& path) {
	return asset_directory_path / path;
}

void free_asset(const std::string& name) {
	if (auto asset = assets.find(name); asset != assets.end()) {
		asset->second.free(asset->second.any);
		asset->second.any = nullptr;
		asset->second.is_loaded = false;
	}
}

void free_released_assets() {
	for (auto& asset : to_be_released) {
		free_asset(asset);
	}
	to_be_released.clear();
}

void register_asset(const std::string& name, const load_asset_function& load, const free_asset_function& free) {
	if (assets.find(name) != assets.end()) {
		WARNING_X("assets", "Asset " << name << " already registered.");
	}
	assets[name] = { load, free, 0, nullptr };
}

void unregister_asset(const std::string& name) {
	if (auto asset = assets.find(name); asset != assets.end()) {
		asset->second.free(asset->second.any);
		assets.erase(asset);
	} else {
		WARNING_X("assets", "Asset " << name << " not registered.");
	}
}

void release_asset(const std::string& name) {
	if (auto asset = assets.find(name); asset != assets.end()) {
		asset->second.holders--;
		if (asset->second.holders <= 0 && asset->second.is_loaded) {
			to_be_released.insert(name);
		}
	}
}

void register_texture(const std::string& name) {
	register_asset("textures/" + name, [name]() -> void* {
		return (void*)create_texture({ asset_path("textures/" + name + ".png") });
	}, [](void* data) {
		delete_texture(reinterpret_cast<int>(data));
	});
}

void register_all_textures() {
	for (const auto& texture : std::filesystem::recursive_directory_iterator{ asset_path("textures") }) {
		register_texture(texture.path().stem().string());
	}
}

scoped_context<int> require_texture(const std::string& name) {
	return { [name] {
		release_asset("textures/" + name);
	}, require_asset<int>("textures/" + name) };
}

void register_font(const std::string& name, int size) {
	register_asset("fonts/" + name + " " + std::to_string(size), [name, size]() -> void* {
		return new font{ name, size };
	}, [](void* data) {
		delete data;
	});
}

scoped_context<font*> require_font(const std::string& name, int size) {
	const std::string full_name{ "fonts/" + name + " " + std::to_string(size) };
	return { [full_name] {
		release_asset(full_name);
	}, require_asset<font*>(full_name) };
}

void register_shader(const std::string& name) {
	register_asset("shaders/" + name, [name]() -> void* {
		return reinterpret_cast<void*>(create_shader({ asset_path("shaders/" + name) }));
	}, [](void* data) {
		delete_shader(reinterpret_cast<int>(data));
	});
}

scoped_context<int> require_shader(const std::string& name) {
	return { [name] {
		release_asset("shaders/" + name);
	}, require_asset<int>("shaders/" + name) };
}

void register_sound(const std::string& name) {
	register_asset("sounds/" + name, [name]() -> void* {
		return new ogg_vorbis_audio_source{ asset_path("sounds/" + name + ".ogg") };
	}, [](void* data) {
		delete static_cast<ogg_vorbis_audio_source*>(data);
	});
}

scoped_context<audio_source*> require_sound(const std::string& name) {
	return { [name] {
		release_asset("sounds/" + name);
	}, require_asset<audio_source*>("sounds/" + name) };
}

}
