export module nfwk.core:datetime;

import std.core;

export namespace nfwk {

template<typename Unit>
auto current_time() {
	return std::chrono::duration_cast<Unit>(std::chrono::system_clock::now().time_since_epoch()).count();
}

}
