#pragma once

#include "loop.hpp"

namespace no::internal {
void initialize_editor();
}

namespace no {

class abstract_editor {
public:

	abstract_editor() = default;
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

};

class editor_state : public program_state {
public:

	editor_state();
	~editor_state() override;

	void update() override;
	void draw() override;

	void open(std::unique_ptr<abstract_editor> editor);
	void close(abstract_editor& editor);

private:

	std::vector<std::unique_ptr<abstract_editor>> editors;

};

void register_editor(std::string_view title, std::function<std::unique_ptr<abstract_editor>()> make_editor);

template<typename Editor>
void register_editor() {
	register_editor(Editor::title, [] {
		return std::make_unique<Editor>();
	});
}

}
