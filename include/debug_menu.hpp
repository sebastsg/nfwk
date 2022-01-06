#pragma once

#include <functional>
#include <unordered_set>
#include <unordered_map>

namespace nfwk {
class loop;
}

namespace nfwk::ui {

class main_menu_bar {
public:

	bool visible{ true };

	main_menu_bar(loop& loop);
	main_menu_bar(const main_menu_bar&) = delete;
	main_menu_bar(main_menu_bar&&) = delete;

	~main_menu_bar() = default;

	main_menu_bar& operator=(const main_menu_bar&) = delete;
	main_menu_bar& operator=(main_menu_bar&&) = delete;

	void add(std::string_view id, std::string_view name, std::function<void()> update);
	void add(std::string_view id, std::function<void()> update);
	void remove(std::string_view id);

	void update();

private:

	class user_entry {
	public:

		std::string id;
		std::function<void()> update;

	};

	loop& owning_loop;
	bool limit_fps{ true };
	bool show_fps{ true };
	bool show_imgui_demo{ false };
	std::unordered_map<std::string, std::vector<user_entry>> user_entry_groups;

};

class integrated_log_ui {
public:

	integrated_log_ui(std::shared_ptr<main_menu_bar> menu_bar);
	integrated_log_ui(const integrated_log_ui&) = delete;
	integrated_log_ui(integrated_log_ui&&) = delete;

	~integrated_log_ui();

	integrated_log_ui& operator=(const integrated_log_ui&) = delete;
	integrated_log_ui& operator=(integrated_log_ui&&) = delete;

	void update();

private:
	
	std::unordered_set<std::string> open_log_windows;
	std::shared_ptr<main_menu_bar> menu_bar;
	
};

}
