export module nfwk.core:event_queue;

import std.core;
import :events;

export namespace nfwk {

template<typename... T>
class event_queue {
public:

	event_queue() = default;
	event_queue(const event_queue&) = delete;

	event_queue(event_queue&& that) noexcept : messages{ std::move(that.messages) } {}

	event_queue& operator=(const event_queue&) = delete;

	event_queue& operator=(event_queue&& that) noexcept {
		std::swap(messages, that.messages);
		return *this;
	}

	template<typename... Args>
	void emplace(Args... args) {
		messages.emplace(std::forward<Args>(args)...);
	}

	void all(const std::function<void(T...)>& handler) {
		if (handler) {
			while (!messages.empty()) {
				std::apply(handler, messages.front());
				messages.pop();
			}
		}
	}

	void emit(const event<T...>& event_) {
		while (!messages.empty()) {
			std::apply([&](auto&&... args) {
				event_.emit(std::forward<T>(args)...);
			}, messages.front());
			messages.pop();
		}
	}

	std::size_t size() const {
		return messages.size();
	}

private:

	std::queue<std::tuple<T...>> messages;

};

}
