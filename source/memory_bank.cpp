#include "memory_bank.hpp"

namespace nfwk {

memory_bank& memory_bank::for_current_thread() {
	thread_local memory_bank bank;
	return bank;
}

std::shared_ptr<io_stream> memory_bank::borrow() {
	for (auto& stream : streams) {
		if (stream.use_count() == 1) {
			stream->reset();
			return stream;
		}
	}
	return streams.emplace_back(std::make_shared<io_stream>());
}

}
