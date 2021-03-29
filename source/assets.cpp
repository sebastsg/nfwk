#include "assets.hpp"
#include "log.hpp"

namespace nfwk {

asset_wrapper_base::~asset_wrapper_base() {}

asset_manager::asset_manager(const std::filesystem::path& directory_path) : directory_path{ directory_path } {
	info(core::log, u8"Asset directory: {}", directory_path);
}

std::any asset_manager::find(const std::type_info& type_info, std::u8string_view name) {
	for (auto& asset : assets) {
		if (asset->name == name && asset->get_type_info() == type_info) {
			return asset->get();
		}
	}
	warning(core::log, u8"Asset not found: {}", name);
	return {};
}

void asset_manager::remove(const std::type_info& type_info, const std::u8string& name) {
	for (std::size_t i{ 0 }; i < assets.size(); i++) {
		if (assets[i]->name == name && assets[i]->get_type_info() == type_info) {
			assets.erase(assets.begin() + i);
			return;
		}
	}
	warning(core::log, u8"Asset {} not defined.", name);
}

std::filesystem::path asset_manager::get_directory() const {
	return directory_path;
}

std::filesystem::path asset_manager::path(std::u8string_view asset) const {
	return directory_path / asset;
}

std::u8string asset_wrapper_base::get_name(std::filesystem::path path) const {
	path.replace_extension();
	auto name = path.u8string();
	if (const auto root = manager.get_directory().u8string(); name.size() > root.size() + 1) {
		name.erase(0, root.size() + 1);
	}
	std::replace(name.begin(), name.end(), '\\', '/');
	return name;
}

}
