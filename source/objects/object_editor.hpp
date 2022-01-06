#pragma once

#include "editor.hpp"
#include "objects/objects.hpp"

#include <optional>

namespace nfwk::script {

class script_event_editor : public ui::window_wrapper {
public:

	script_event_editor(script_event_container& event_container);

	void update(ui::window_container& container) override;
	bool has_unsaved_changes() const override;

	std::string get_name() const override;

	int get_flags() const override {
		return 0;
	}

private:

	int selected_event_index{ 0 };
	std::string new_event_id;
	bool docked{ false };
	bool dirty{ false };
	script_event_container& event_container;

};

class object_class_editor : public ui::window_wrapper {
public:

	object_class_editor();
	object_class_editor(std::shared_ptr<object_class> definition);

	void update(ui::window_container& container) override;
	void save();

	bool has_unsaved_changes() const override;

	std::string get_name() const override;

private:

	std::shared_ptr<object_manager> objects;

	std::shared_ptr<object_class> definition;
	bool dirty{ false };
	std::optional<std::string> new_id;

	std::string new_variable_name;
	variable_type new_variable_type{ variable_type::string };
	int selected_variable{ 0 };

	std::string new_event_id;
	int selected_event{ 0 };

};

class object_class_list_editor : public ui::window_wrapper {
public:
	
	void update(ui::window_container& container) override;

	std::string get_name() const override;

private:

	std::shared_ptr<object_manager> objects;

};

}
