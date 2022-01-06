#pragma once

#include "io.hpp"

namespace nfwk {

class memory_bank {
public:

	[[nodiscard]] static memory_bank& for_current_thread();
	
	[[nodiscard]] std::shared_ptr<io_stream> borrow();

private:

	std::vector<std::shared_ptr<io_stream>> streams;
	
};

}
