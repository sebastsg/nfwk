#pragma once

#include "editor.hpp"
#include "objects.hpp"

#include <optional>

namespace no {

class object_class_editor : public abstract_editor {
public:

	static constexpr std::string_view title{ "Object class" };

	object_class_editor();
	object_class_editor(const object_class_definition& definition);

	void update() override;
	void save();

	std::string_view get_title() const override {
		return title;
	}

	bool is_dirty() const override {
		return dirty;
	}

private:

	object_class_definition definition;
	bool dirty{ false };
	std::optional<std::string> new_id;

};

class object_class_list_editor : public abstract_editor {
public:

	static constexpr std::string_view title{ "Object class list" };

	void update() override;

	std::string_view get_title() const override {
		return title;
	}

	bool is_dirty() const override {
		return false;
	}

};

}
