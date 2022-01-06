#pragma once

#include "subprogram.hpp"
#include "graphics/ui.hpp"

namespace nfwk::ui {

class window_container;
class texture;

enum class docking_direction { left, right, up, down, none };

class window_wrapper {
public:

	friend class window_container;
	
	bool open{ false };

	window_wrapper() = default;
	window_wrapper(const window_wrapper&) = delete;
	window_wrapper(window_wrapper&&) = default;

	virtual ~window_wrapper() = default;

	window_wrapper& operator=(const window_wrapper&) = delete;
	window_wrapper& operator=(window_wrapper&&) = delete;

	virtual void update(window_container& container) = 0;
	virtual void after_update() {}

	virtual std::string get_name() const = 0;

	virtual std::string get_unique_id() const {
		return {};
	}
	
	virtual int get_flags() const {
		return default_window_flags;
	}
	
	virtual bool has_unsaved_changes() const {
		return false;
	}

	virtual void save() {}
	
	bool is_open() const {
		return open;
	}

	void dock(docking_direction direction, float ratio);

private:

	scoped_logic begin_update();

	docking_direction direction{ docking_direction::none };
	float ratio{ 0.0f };
	bool needs_to_dock{ false };
	
};

class window_container {
public:

	window_container();
	window_container(const window_container&) = delete;
	window_container(window_container&&) = delete;
	
	~window_container();

	window_container& operator=(const window_container&) = delete;
	window_container& operator=(window_container&&) = delete;
	
	void update();
	void dock(docking_direction direction, float ratio);

	void open(std::shared_ptr<window_wrapper> window);
	void close(window_wrapper& window);
	
private:

	std::shared_ptr<texture> blank_texture;
	std::vector<std::shared_ptr<window_wrapper>> windows;
	ImGuiWindow* root_window{ nullptr };
	ImGuiDockNode* dock_node{ nullptr };

};

/*class add_script_event_editor : public window_wrapper {
public:

	void update(window_container& container) override;
	std::string_view get_title() const override;
	bool has_unsaved_changes() const override;

};*/

}
