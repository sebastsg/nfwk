module;

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

export module nfwk.ui:game_event_editor;

import std.core;
import nfwk.core;
import nfwk.scripts;
import :imgui_wrapper;
import :editor;

export namespace nfwk {

class game_event_editor : public abstract_editor {
public:

	static constexpr std::string_view title{ "Events" };

	game_event_editor() = default;

	~game_event_editor() override = default;

	void update() override {
		auto& container = game_event_container::global();
		// TODO:
		// title + "##" + this
		if (auto _ = ui::window(title, 0, &open)) {
			if (auto editor = static_cast<editor_state*>(program_state::current()); editor && !docked) {
				editor->dock(ImGuiDir_None, 0.0f);
				docked = true;
			}
			const auto& events = container.get();
			if (auto new_selection = ui::list("##event-list", vector_to_strings(events), selected_event_index, 10)) {
				selected_event_index = new_selection.value();
			}
			if (!events.empty()) {
				ui::inline_next();
				if (ui::button("Delete")) {
					container.remove(events[selected_event_index]->get_id());
					if (selected_event_index >= static_cast<int>(events.size())) {
						selected_event_index--;
					}
					dirty = true;
				}
			}
			ui::separate();
			ui::input("New Event", new_event_id);
			new_event_id = normalized_identifier(new_event_id);
			ui::inline_next();
			if (auto _ = ui::disable_if(container.exists(new_event_id))) {
				if (ui::button("Add")) {
					container.add(new_event_id);
					new_event_id = "";
				}
			}
			ui::separate();
			if (auto _ = ui::disable_if(!is_dirty())) {
				if (ui::button("Save")) {
					container.save();
				}
			}
		}
	}

	bool is_dirty() const override {
		return dirty;
	}

private:

	int selected_event_index{ 0 };
	std::string new_event_id;
	bool docked{ false };
	bool dirty{ false };

};

}
