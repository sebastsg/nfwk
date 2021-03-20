#pragma once

#include "platform.hpp"
#include "windows_platform.hpp"
#include "graphics/window.hpp"
#include "input.hpp"
#include "graphics/gl/wgl_context.hpp"

namespace nfwk::platform {

class windows_window : public window {
public:
	
	static void create_classes();
	static std::shared_ptr<render_context> create_compatibility_render_context();

	windows_window(std::string_view title, std::optional<vector2i> size = std::nullopt);
	~windows_window() override;

	void poll() override;
	void maximize() override;

	std::shared_ptr<render_context> create_render_context(std::optional<int> samples) const override;

	bool is_open() const override;
	vector2i position() const override;
	vector2i size() const override;
	std::string title() const override;

	void set_title(std::string_view title) override;
	void set_size(vector2i size) override;

	void swap() override;

	void set_display_mode(display_mode mode) override;
	display_mode current_display_mode() const override;

	HWND handle() const;
	HDC get_device_context() const;
	void set_icon_from_resource(int resource_id);

private:
	
	static HWND create_window(std::string_view name, std::string_view type, int width, int height, bool maximized);

	void set_data();
	void show(bool maximized);

	HWND window_handle{ nullptr };
	HDC device_context{ nullptr };
	display_mode last_set_display_mode{ display_mode::windowed };

};

}
