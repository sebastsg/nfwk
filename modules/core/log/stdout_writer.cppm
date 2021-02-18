export module nfwk.core:log.stdout_writer;

import std.core;
import :log.core;
import :events;

export namespace nfwk::log {

class stdout_writer : public log_writer {
public:

	stdout_writer(debug_log& log) : log{ log } {
		for (const auto& entry : log.get_entries()) {
			write(entry);
		}
		new_entry_event = log.on_new_entry.listen([this](const log_entry& entry) {
			write(entry);
		});
	}

private:

	void write(const log_entry& entry) {
		if (log.show_time) {
			std::cout << std::left << std::setw(13) << current_time_string_for_log() << std::setw(1) << std::internal;
		}
		if (log.show_file) {
			std::cout << entry.file << ": ";
		}
		if (log.show_line) {
			std::cout << std::setw(4) << entry.line << ": " << std::setw(1);
		}
		std::cout << entry.type << ": " << entry.message << "\n";
	}

	event_listener new_entry_event;
	debug_log& log;

};

}
