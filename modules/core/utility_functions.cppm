export module nfwk.core:utility_functions;

import std.core;
import std.threading;

export namespace nfwk {

template<typename T>
bool is_future_ready(const std::future<T>& future) {
	return future.valid() && future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

template<typename T>
std::vector<T> merge_vectors(const std::vector<T>& front, const std::vector<T>& back) {
	std::vector<T> result;
	result.reserve(front.size() + back.size());
	result.insert(result.end(), front.begin(), front.end());
	result.insert(result.end(), back.begin(), back.end());
	return result;
}

template<typename T>
struct is_pointer {
	static constexpr bool value{ false };
};

template<typename T>
struct is_pointer<T*> {
	static constexpr bool value{ true };
};

}
