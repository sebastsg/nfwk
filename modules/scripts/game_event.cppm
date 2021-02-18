module;

#include "assert.hpp"

export module nfwk.scripts:events;

import std.core;
import nfwk.core;
import nfwk.assets;
import :script_utility_functions;

export namespace nfwk {

class game_event_container;

class game_event {
public:
	
	game_event(game_event_container& container, io_stream& stream) : container{ &container } {
		read(stream);
	}

	game_event(game_event_container& container, std::string_view id) : container{ &container }, id{ id } {
		ASSERT(is_identifier_normalized(id));
	}

	game_event(const game_event&) = delete;
	game_event(game_event&&) = default;

	~game_event() = default;

	game_event& operator=(const game_event&) = delete;
	game_event& operator=(game_event&&) = default;

	std::string to_string() const {
		return get_id();
	}

	std::string get_id() const;

	void trigger() const {
		event.emit();
		for (const auto& script_id : scripts) {
			//run_script(script_id);
		}
	}

	void attach_script(const std::string& script_id) {
		ASSERT(!has_script(script_id));
		scripts.push_back(script_id);
	}

	void detach_script(const std::string& script_id) {
		std::erase(scripts, script_id);
	}

	bool has_script(std::string_view script_id) const {
		return std::find(scripts.begin(), scripts.end(), script_id) != scripts.end();
	}

	void write(io_stream& stream) const {
		ASSERT(is_identifier_normalized(id));
		stream.write(id);
		stream.write_array<std::string>(scripts);
	}
	
	void read(io_stream& stream) {
		id = stream.read<std::string>();
		scripts = stream.read_array<std::string>();
		ASSERT(is_identifier_normalized(id));
	}

private:

	std::string id;
	std::vector<std::string> scripts;
	event<> event;
	game_event_container* container{ nullptr };

};

class game_event_container {
public:
	
	game_event_container(std::string_view id) : id{ id } {
		load();
	}

	game_event_container() = default;

	std::string get_id() const {
		return id;
	}

	void set_id(std::string_view new_id) {
		id = new_id;
		ASSERT(is_identifier_normalized(id));
	}

	[[nodiscard]] std::vector<game_event*> get() {
		std::vector<game_event*> result;
		for (auto& event : events) {
			result.push_back(&event);
		}
		return result;
	}

	[[nodiscard]] std::vector<game_event*> with_script(std::string_view script_id) {
		std::vector<game_event*> result;
		for (auto& event : events) {
			if (event.has_script(script_id)) {
				result.push_back(&event);
			}
		}
		return result;
	}

	game_event* add(std::string_view event_id) {
		if (exists(event_id)) {
			return nullptr;
		} else {
			return &events.emplace_back(*this, event_id);
		}
	}

	void remove(std::string_view event_id) {
		std::erase_if(events, [&event_id](const auto& event) {
			return event.get_id() == event_id;
		});
	}

	game_event* find(std::string_view event_id) {
		auto event = std::find_if(events.begin(), events.end(), [&event_id](const auto& event) {
			return event.get_id() == event_id;
		});
		if (event == events.end()) {
			event = std::find_if(events.begin(), events.end(), [this, &event_id](const auto& event) {
				return event.get_id() == id + ":" + std::string { event_id};
			});
		}
		return event != events.end() ? &*event : nullptr;
	}

	void trigger(std::string_view event_id) {
		if (auto event = find(event_id)) {
			event->trigger();
		} else {
			info("game", "{} does not exist.", event_id);
		}
	}

	bool exists(std::string_view event_id) const {
		return events.end() != std::find_if(events.begin(), events.end(), [&event_id](const auto& event) {
			return event.get_id() == event_id;
		});
	}

	void write(io_stream& stream) const {
		ASSERT(is_identifier_normalized(id));
		stream.write(id);
		stream.write(static_cast<std::int32_t>(events.size()));
		for (const auto& event : events) {
			event.write(stream);
		}
	}

	void read(io_stream& stream) {
		events.clear();
		id = stream.read<std::string>();
		const auto count = stream.read<std::int32_t>();
		for (int i{ 0 }; i < count; i++) {
			events.emplace_back(*this, stream);
		}
		ASSERT(is_identifier_normalized(id));
	}

	void save() const {
		io_stream stream;
		write(stream);
		write_file(asset_path(id + ".nfwk-events"), stream);
	}

	void load() {
		if (io_stream stream{ asset_path(id + ".nfwk-events") }; !stream.empty()) {
			read(stream);
		}
	}

	[[nodiscard]] std::vector<game_event*> search(const std::string& search_term, int limit) {
		std::vector<game_event*> results;
		const auto& search = string_to_lowercase(search_term);
		for (auto& event : events) {
			const auto& event_id = string_to_lowercase(event.get_id());
			if (event_id.find(search) != std::string::npos) {
				results.push_back(&event);
			}
			if (static_cast<int>(results.size()) >= limit) {
				break;
			}
		}
		return results;
	}

	static game_event_container& global() {
		static game_event_container container{ "global" };
		return container;
	}

private:

	std::vector<game_event> events;
	std::string id;

};

std::string game_event::get_id() const {
	return container ? container->get_id() + ":" + id : id;
}

#if 0
std::vector<const game_event*> search_object_game_events(const std::string& search_term, int limit) {
	std::vector<const game_event*> results;
	const auto& lowercase_search_term = string_to_lowercase(search_term);
	std::optional<std::string> class_id_search;
	auto event_id_search = lowercase_search_term;
	if (const auto class_id_separator = event_id_search.find(':'); class_id_separator != std::string::npos) {
		class_id_search = lowercase_search_term.substr(0, class_id_separator);
		event_id_search = lowercase_search_term.substr(class_id_separator + 1);
	}
	for (const auto& event : get_object_game_events()) {
		const auto& class_index = event.get_parent_id().value();
		const auto& class_id = objects::get_class_id(class_index).value_or("");
		const auto& event_id = string_to_lowercase(event.get_id());
		const bool event_matches{ event_id.find(event_id_search) != std::string::npos };
		const bool class_matches{ !class_id_search.has_value() || class_id.find(class_id_search.value()) != std::string::npos };
		if (event_matches && class_matches) {
			results.push_back(&event);
		}
		if (static_cast<int>(results.size()) >= limit) {
			break;
		}
	}
	return results;
}
#endif

}
