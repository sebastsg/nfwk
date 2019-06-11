#include "assets.hpp"
#include "debug.hpp"
#include "draw.hpp"
#include "surface.hpp"
#include "font.hpp"

#include <unordered_map>
#include <unordered_set>
#include <filesystem>

namespace no {

struct registered_asset {
	load_asset_func load;
	free_asset_func free;
	int holders = 0;
	void* any = nullptr;
	bool is_loaded = false;
};

static std::string asset_directory_path = "assets";
static std::unordered_map<std::string, registered_asset> assets;
static std::unordered_set<std::string> to_be_released;

namespace internal {

void* require_asset(const std::string& name) {
	auto asset = assets.find(name);
	if (asset == assets.end()) {
		return nullptr;
	}
	asset->second.holders++;
	if (asset->second.is_loaded) {
		to_be_released.erase(name);
	} else {
		asset->second.any = asset->second.load();
		asset->second.is_loaded = true;
	}
	return asset->second.any;
}

}

void set_asset_directory(const std::string& path) {
	asset_directory_path = path;
}

std::string asset_directory() {
	return asset_directory_path;
}

std::string asset_path(const std::string& path) {
	return asset_directory_path + "/" + path;
}

void free_asset(const std::string& name) {
	auto asset = assets.find(name);
	if (asset == assets.end()) {
		return;
	}
	asset->second.free(asset->second.any);
	asset->second.any = nullptr;
	asset->second.is_loaded = false;
}

void free_released_assets() {
	for (auto& asset : to_be_released) {
		free_asset(asset);
	}
	to_be_released.clear();
}

void register_asset(const std::string& name, const load_asset_func& load, const free_asset_func& free) {
	if (assets.find(name) != assets.end()) {
		WARNING("Asset " << name << " already registered.");
	}
	assets[name] = { load, free, 0, nullptr };
}

void unregister_asset(const std::string& name) {
	auto asset = assets.find(name);
	if (asset == assets.end()) {
		WARNING("Asset " << name << " not registered.");
	}
	asset->second.free(asset->second.any);
	assets.erase(asset);
}

void release_asset(const std::string& name) {
	auto asset = assets.find(name);
	if (asset == assets.end()) {
		return;
	}
	asset->second.holders--;
	if (asset->second.holders <= 0 && asset->second.is_loaded) {
		to_be_released.insert(name);
	}
}

void register_texture(const std::string& name) {
	register_asset("textures/" + name, [name]() -> void* {
		return (void*)create_texture({ no::asset_path("textures/" + name + ".png") });
	}, [](void* data) {
		delete_texture((int)data);
	});
}

void register_all_textures() {
	std::filesystem::recursive_directory_iterator textures{ no::asset_path("textures") };
	for (auto& texture : textures) {
		register_texture(texture.path().stem().string());
	}
}

int require_texture(const std::string& name) {
	return require_asset<int>("textures/" + name);
}

void release_texture(const std::string& name) {
	release_asset("textures/" + name);
}

void register_font(const std::string& name, int size) {
	register_asset("fonts/" + name + " " + std::to_string(size), [name, size]() -> void* {
		return new font{ no::asset_path("fonts/" + name + ".ttf"), size };
	}, [](void* data) {
		delete data;
	});
}

font* require_font(const std::string& name, int size) {
	return require_asset<font*>("fonts/" + name + " " + std::to_string(size));
}

void release_font(const std::string& name, int size) {
	release_asset("fonts/" + name + " " + std::to_string(size));
}

void register_shader(const std::string& name) {
	register_asset("shaders/" + name, [name]() -> void* {
		return (void*)create_shader({ no::asset_path("shaders/" + name) });
	}, [](void* data) {
		delete_shader((int)data);
	});
}

int require_shader(const std::string& name) {
	return require_asset<int>("shaders/" + name);
}

void release_shader(const std::string& name) {
	release_asset("shaders/" + name);
}

}
