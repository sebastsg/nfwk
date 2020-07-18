#pragma once

#include <functional>
#include <optional>

namespace no {

class [[nodiscard]] scoped_logic {
public:

	scoped_logic(std::function<void()> clean = {}) : clean{ std::move(clean) } {

	}

	scoped_logic(const scoped_logic&) = delete;

	scoped_logic(scoped_logic&& that) noexcept : clean{ std::move(that.clean) } {

	}

	scoped_logic& operator=(const scoped_logic&) = delete;

	scoped_logic& operator=(scoped_logic&& that) noexcept {
		std::swap(clean, that.clean);
		return *this;
	}

	~scoped_logic() {
		if (clean) {
			clean();
		}
	}

	operator bool() const {
		return clean.operator bool();
	}

private:

	std::function<void()> clean;

};

template<typename T>
class [[nodiscard]] scoped_context {
public:

	scoped_context(std::function<void()> clean = {}, std::optional<T> resource = std::nullopt)
		: scope{ std::move(clean) }, resource{ std::move(resource) } {

	}

	scoped_context(const scoped_context&) = delete;

	scoped_context(scoped_context&& that) noexcept
		: scope{ std::move(that.scope) }, resource{ std::move(that.resource) } {

	}

	scoped_context& operator=(const scoped_context&) = delete;

	scoped_context& operator=(scoped_context&& that) noexcept {
		std::swap(scope, that.scope);
		std::swap(resource, that.resource);
		return *this;
	}

	operator bool() const {
		return scope.operator bool() && resource.has_value();
	}

	const std::optional<T>& get() const {
		return resource;
	}

	std::optional<T>& get() {
		return resource;
	}

	operator T& () {
		return resource.value();
	}

private:

	scoped_logic scope;
	std::optional<T> resource;

};

}
