#pragma once

#include "log.hpp"

namespace nfwk::log {

class stdout_writer : public log_writer {
public:

	stdout_writer(std::shared_ptr<debug_log> log);

private:

	void write(const log_entry& entry);

	event_listener new_entry_event;

};

}
