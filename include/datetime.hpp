#pragma once

#include <chrono>

namespace nfwk {

template<typename Unit>
auto current_time() {
	return std::chrono::duration_cast<Unit>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

}
