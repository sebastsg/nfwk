#pragma once

#include "editor.hpp"
#include "objects/objects.hpp"

#include <optional>

namespace no {

class object_class_editor : public abstract_editor {
public:

	static constexpr std::string_view title{ "Object class" };

	object_class_editor();
	object_class_editor(const object_class& definition);

	void update() override;
	void save();

	std::string_view get_title() const override;
	bool is_dirty() const override;

private:

	object_class definition;
	bool dirty{ false };
	std::optional<std::string> new_id;

	std::string new_variable_name;
	variable_type new_variable_type{ variable_type::string };
	int selected_variable{ 0 };

	std::string new_event_id;
	int selected_event{ 0 };

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
