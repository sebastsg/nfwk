#pragma once

namespace nfwk::return_code {
enum {
	// General (0-999)
	success = 0,
	unknown_error = 1,

	// Windows (1000-1999)
	windows_com_initialize_failed = 1000
};
}

void start();
