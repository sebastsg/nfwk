#pragma once

#include "subprogram.hpp"
#include "imgui_loop_component.hpp"

namespace nfwk {

class editor_state;

class abstract_editor {
public:

	abstract_editor(editor_state& editor) : editor{ editor } {}
	abstract_editor(const abstract_editor&) = delete;
	abstract_editor(abstract_editor&&) = default;

	virtual ~abstract_editor() = default;

	abstract_editor& operator=(const abstract_editor&) = delete;
	abstract_editor& operator=(abstract_editor&&) = default;

	virtual void update() = 0;
	virtual std::string_view get_title() const = 0;
	virtual bool is_dirty() const = 0;

	bool is_open() const {
		return open;
	}

protected:

	bool open{ true };
	editor_state& editor;

};

class add_game_event_editor : public abstract_editor {
public:

	add_game_event_editor(editor_state& editor) : abstract_editor{ editor } {}

	void update() override;
	std::string_view get_title() const override;
	bool is_dirty() const override;

};

class editor_state : public subprogram {
public:

	std::shared_ptr<nfwk::window> window;

	editor_state(loop& loop);
	~editor_state() override;

	void update() override;

	void open(std::unique_ptr<abstract_editor> editor);
	void close(abstract_editor& editor);

	void register_editor(std::string_view title, std::function<std::unique_ptr<abstract_editor>()> make_editor);

	template<typename Editor>
	void register_editor() {
		register_editor(Editor::title, [this] {
			return std::make_unique<Editor>(*this);
		});
	}

private:

	std::vector<std::unique_ptr<abstract_editor>> editors;
	std::vector<std::pair<std::string, std::function<std::unique_ptr<abstract_editor>()>>> editor_makers;

};

}
