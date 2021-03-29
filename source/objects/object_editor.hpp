#pragma once

#include "editor.hpp"
#include "objects/objects.hpp"

#include <optional>

namespace nfwk {

class script_event_editor : public abstract_editor {
public:

	static constexpr std::u8string_view title{ u8"Script events" };

	script_event_editor(editor_container& container, script_event_container& event_container);

	void update() override;
	bool is_dirty() const override;

private:

	int selected_event_index{ 0 };
	std::u8string new_event_id;
	bool docked{ false };
	bool dirty{ false };
	script_event_container& event_container;

};

class object_class_editor : public abstract_editor {
public:

	static constexpr std::u8string_view title{ u8"Object class" };

	object_class_editor(editor_container& container);
	object_class_editor(editor_container& container, const object_class& definition);

	void update() override;
	void save();

	std::u8string_view get_title() const override;
	bool is_dirty() const override;

private:

	std::shared_ptr<object_manager> objects;

	object_class definition;
	bool dirty{ false };
	std::optional<std::u8string> new_id;

	std::u8string new_variable_name;
	variable_type new_variable_type{ variable_type::string };
	int selected_variable{ 0 };

	std::u8string new_event_id;
	int selected_event{ 0 };

};

class object_class_list_editor : public abstract_editor {
public:

	using abstract_editor::abstract_editor;

	static constexpr std::u8string_view title{ u8"Object class list" };

	void update() override;

	std::u8string_view get_title() const override {
		return title;
	}

	bool is_dirty() const override {
		return false;
	}

private:

	std::shared_ptr<object_manager> objects;

};

}
