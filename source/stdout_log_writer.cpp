#include "stdout_log_writer.hpp"

#include <iostream>

namespace nfwk::log {

stdout_writer::stdout_writer(std::shared_ptr<debug_log> log_) : log_writer{ std::move(log_) } {
	for (const auto& entry : log->get_entries()) {
		write(entry);
	}
	new_entry_event = log->on_new_entry.listen([this](const log_entry& entry) {
		write(entry);
	});
}

void stdout_writer::write(const log_entry& entry) {
	if (log->show_time) {
		std::cout << std::left << std::setw(13) << current_time_string_for_log() << std::setw(1) << std::internal;
	}
	if (log->show_file) {
		std::cout << entry.file << ": ";
	}
	if (log->show_line) {
		std::cout << std::setw(4) << entry.line << ": " << std::setw(1);
	}
	std::cout << entry.type << ": " << entry.message << "\n";
}

}
