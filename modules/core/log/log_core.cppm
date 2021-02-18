export module nfwk.core:log.core;

import std.core;
import :events;
export import :log.entry;

export namespace nfwk::log {

class debug_log {
public:

	event<const log_entry&> on_new_entry;

	const std::string name;

	bool show_time{ true };
	bool show_file{ true };
	bool show_line{ true };

	debug_log(std::string_view name) : name{ name } {}

	int count() const {
		return static_cast<int>(entries.size());
	}

	const std::vector<log_entry>& get_entries() const {
		return entries;
	}

	template<typename... Args>
	void add(Args&&... args) {
		std::lock_guard lock{ mutex };
		const auto& entry = entries.emplace_back(std::forward<Args>(args)...);
		on_new_entry.emit(entry);
	}

private:

	std::vector<log_entry> entries;
	std::mutex mutex;

};

class log_writer {
public:

	virtual ~log_writer() = default;

	virtual void open() const {}

};

}
