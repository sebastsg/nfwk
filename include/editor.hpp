#pragma once

#include "subprogram.hpp"
#include "imgui_loop_component.hpp"
#include "graphics/ui.hpp"

namespace nfwk {

class editor_container;
class texture;

class abstract_editor {
public:

	abstract_editor(editor_container& container);
	abstract_editor(const abstract_editor&) = delete;
	abstract_editor(abstract_editor&&) = default;

	virtual ~abstract_editor() = default;

	abstract_editor& operator=(const abstract_editor&) = delete;
	abstract_editor& operator=(abstract_editor&&) = delete;

	virtual void update() = 0;
	virtual std::u8string_view get_title() const = 0;
	virtual bool is_dirty() const = 0;

	bool is_open() const {
		return open;
	}

protected:

	bool open{ true };
	editor_container& container;

};

class add_script_event_editor : public abstract_editor {
public:

	using abstract_editor::abstract_editor;

	void update() override;
	std::u8string_view get_title() const override;
	bool is_dirty() const override;

};

class editor_container {
public:

	editor_container();
	editor_container(const editor_container&) = delete;
	editor_container(editor_container&&) = delete;
	
	~editor_container();

	editor_container& operator=(const editor_container&) = delete;
	editor_container& operator=(editor_container&&) = delete;
	
	void update();
	void dock(int direction, float ratio);

	void open(std::unique_ptr<abstract_editor> editor);
	void close(abstract_editor& editor);

	void register_editor(std::u8string_view title, std::function<std::unique_ptr<abstract_editor>()> make_editor);

	template<typename Editor>
	void register_editor() {
		register_editor(Editor::title, [this] {
			return std::make_unique<Editor>(*this);
		});
	}

private:

	std::shared_ptr<texture> blank_texture;
	std::vector<std::unique_ptr<abstract_editor>> editors;
	std::vector<std::pair<std::u8string, std::function<std::unique_ptr<abstract_editor>()>>> editor_makers;
	ImGuiWindow* root_window{ nullptr };
	ImGuiDockNode* dock_node{ nullptr };

};

}
