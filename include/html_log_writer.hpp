#pragma once

#include "log.hpp"

namespace nfwk::log {

class html_writer : public log_writer {
public:

	html_writer(std::shared_ptr<debug_log> log);

	void open() const override;
	void flush() override;

private:

	static std::string field_html(const std::string& message, int col_span = 1);
	static std::string entry_html(const log_entry& entry);
	static std::string html_compatible_string(std::string string);

	std::string buffer;
	std::filesystem::path path;
	bool first_flush{ true };
	event_listener new_entry_event;

};

}