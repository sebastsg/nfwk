#include "presets.hpp"

namespace nfwk {

window_manager& get_window_manager(subprogram& subprogram) {
	thread_local std::unique_ptr<window_manager> windows;
	if (!windows) {
		windows = std::make_unique<window_manager>(subprogram.get_loop());
	}
	return *windows;
}

std::unique_ptr<asset_manager> make_asset_manager(const std::filesystem::path& directory) {
	auto assets = std::make_unique<asset_manager>(directory);
	assets->preload_type<texture_asset>();
	assets->preload_type<font_asset>();
	assets->preload_type<shader_asset>();
	return assets;
}

std::shared_ptr<window> make_window(subprogram& subprogram, std::string_view title, std::optional<vector2i> size) {
	auto window = get_window_manager(subprogram).create_window(title, size);
	window->attach_subprogram(subprogram);
	return window;
}

}
