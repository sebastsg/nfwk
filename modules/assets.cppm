export module nfwk.assets;

import std.core;
import std.filesystem;

export namespace nfwk {

std::filesystem::path asset_path(const std::string& path) {
	return path;
}

}

#if 0
namespace nfwk {

static void* const invalid_asset_handle{ reinterpret_cast<void*>(-1) };

static std::filesystem::path asset_directory_path{ "." };
static std::unordered_map<std::string, registered_asset> assets;
static std::unordered_set<std::string> to_be_released;

static void free_asset(const std::string& name) {
	if (auto asset = assets.find(name); asset != assets.end()) {
		asset->second.free(asset->second.handle);
		asset->second.handle = invalid_asset_handle;
		asset->second.is_loaded = false;
	}
}

static void unregister_asset(const std::string& name) {
	if (auto asset = assets.find(name); asset != assets.end()) {
		asset->second.free(asset->second.handle);
		assets.erase(asset);
	} else {
		warning("assets", "Asset {} not registered.", name);
	}
}

struct registered_asset {
	load_asset_function load;
	free_asset_function free;
	int holders{ 0 };
	void* handle{ invalid_asset_handle };
	bool is_loaded{ false };
};

}

export namespace nfwk {

using load_asset_function = std::function<void*()>;
using free_asset_function = std::function<void(void*)>;

namespace internal {

void* require_asset(const std::string& name) {
	if (auto asset = assets.find(name); asset != assets.end()) {
		asset->second.holders++;
		if (asset->second.is_loaded) {
			to_be_released.erase(name);
		} else {
			asset->second.handle = asset->second.load();
			asset->second.is_loaded = true;
		}
		return asset->second.handle;
	} else {
		return invalid_asset_handle;
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

void register_asset(const std::string& name, const load_asset_function& load, const free_asset_function& free) {
	if (assets.find(name) != assets.end()) {
		warning("assets", "Asset {} already registered.", name);
	}
	assets[name] = { load, free, 0, nullptr };
}

template<typename T>
T require_asset(const std::string& name) {
	return reinterpret_cast<T>(internal::require_asset(name));
}

void release_asset(const std::string& name) {
	if (auto asset = assets.find(name); asset != assets.end()) {
		asset->second.holders--;
		if (asset->second.holders <= 0 && asset->second.is_loaded) {
			to_be_released.insert(name);
		}
	}
}

void free_released_assets() {
	for (auto& asset : to_be_released) {
		free_asset(asset);
	}
	to_be_released.clear();
}

void register_texture(const std::string& name) {
	register_asset("textures/" + name, [name]() -> void* {
		return reinterpret_cast<void*>(create_texture({ asset_path("textures/" + name + ".png") }));
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
	const auto id = require_asset<int>("textures/" + name);
	return { [name] {
		release_asset("textures/" + name);
	}, id != -1 ? id : std::optional<int>{} };
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
#endif