#pragma once

#include "io.hpp"
#include "graphics/surface.hpp"
#include "graphics/font.hpp"
#include "graphics/texture.hpp"
#include "graphics/shader.hpp"
#include "log.hpp"

#include <any>

namespace nfwk {

class asset_manager;

class asset_wrapper_base {
public:

	const asset_manager& manager;
	const std::filesystem::path path;
	const std::u8string name;

	asset_wrapper_base(const asset_manager& manager, const std::filesystem::path& path_)
		: manager{ manager }, path { path_ }, name{ get_name(path_) } {}

	asset_wrapper_base(const asset_wrapper_base&) = delete;
	asset_wrapper_base(asset_wrapper_base&&) = delete;

	virtual ~asset_wrapper_base();

	asset_wrapper_base& operator=(const asset_wrapper_base&) = delete;
	asset_wrapper_base& operator=(asset_wrapper_base&&) = delete;

	[[nodiscard]] virtual std::any get() = 0;

	virtual void load() = 0;
	virtual void unload() {}

	[[nodiscard]] virtual const std::type_info& get_type_info() const = 0;

	static std::vector<std::filesystem::path> get_paths(const std::filesystem::path& path) {
		return entries_in_directory(path, entry_inclusion::only_files, true);
	}

private:
	
	[[nodiscard]] virtual std::u8string get_name(std::filesystem::path path) const;

};

class asset_manager {
public:

	asset_manager(const std::filesystem::path& directory_path);
	asset_manager() = default;
	asset_manager(const asset_manager&) = delete;
	asset_manager(asset_manager&&) = delete;

	~asset_manager() = default;

	asset_manager& operator=(const asset_manager&) = delete;
	asset_manager& operator=(asset_manager&&) = delete;

	std::any find(const std::type_info& type_info, std::u8string_view name);

	template<typename Asset>
	std::shared_ptr<Asset> find(std::u8string_view name) {
		try {
			return std::any_cast<std::shared_ptr<Asset>>(find(typeid(Asset), name));
		} catch (std::bad_any_cast e) {
			error("assets", "Asset={} when finding {}\nstd::bad_any_cast: {}", typeid(Asset).name(), name, e.what());
			return nullptr;
		}
	}

	template<typename Asset>
	void preload(const std::filesystem::path& path) {
		assets.emplace_back(std::make_unique<Asset>(*this, path));
	}

	template<typename Asset>
	void preload_type() {
		for (const auto& path : Asset::get_paths(path(Asset::directory))) {
			preload<Asset>(path);
		}
	}

	void remove(const std::type_info& type_info, const std::u8string& name);

	[[nodiscard]] std::filesystem::path get_directory() const;
	[[nodiscard]] std::filesystem::path path(std::u8string_view asset) const;

private:

	std::filesystem::path directory_path{ "." };
	std::vector<std::unique_ptr<asset_wrapper_base>> assets;

};

template<typename Asset>
class asset_wrapper : public asset_wrapper_base {
public:

	using asset_wrapper_base::asset_wrapper_base;

	~asset_wrapper() override {
		if (is_loaded()) {
			unload();
		}
	}

	[[nodiscard]] std::any get() override final {
		if (!is_loaded()) {
			load();
		}
		return asset;
	}

	const std::type_info& get_type_info() const override {
		return typeid(Asset);
	}

	[[nodiscard]] bool try_unload() {
		if (is_used() || !is_loaded()) {
			return false;
		}
		unload();
		asset = nullptr;
		since_unused = {};
		return true;
	}

	bool is_used() const {
		return asset.use_count() > 1;
	}

	bool is_loaded() const {
		return asset != nullptr;
	}

protected:

	std::shared_ptr<Asset> asset;
	std::chrono::seconds since_unused;

};

class texture_asset : public asset_wrapper<texture> {
public:

	static constexpr std::u8string_view directory{ u8"textures" };

	using asset_wrapper::asset_wrapper;

	void load() override {
		asset = std::make_shared<texture>(path);
	}

};

class font_asset : public asset_wrapper<font> {
public:

	static constexpr std::u8string_view directory{ u8"fonts" };

	using asset_wrapper::asset_wrapper;

	void load() override {
		// todo: the font class itself should have a "load_size(int size)" function, so we don't load same asset twice.
		int size{ 18 };
		asset = std::make_shared<font>(path, size);
	}

};

class shader_asset : public asset_wrapper<shader> {
public:

	static constexpr std::u8string_view directory{ u8"shaders" };

	using asset_wrapper::asset_wrapper;

	void load() override {
		asset = std::make_shared<shader>(path);
	}

	static std::vector<std::filesystem::path> get_paths(const std::filesystem::path& path) {
		return entries_in_directory(path, entry_inclusion::only_directories, false);
	}

};

/*class audio_asset : public asset_wrapper<ogg_vorbis_audio_source> {
public:

	static constexpr std::string_view directory{ "sounds" };

	using asset_wrapper::asset_wrapper;

	void load() override {
		asset = std::make_shared<ogg_vorbis_audio_source>(path);
	}

};*/

}
