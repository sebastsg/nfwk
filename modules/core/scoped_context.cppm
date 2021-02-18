export module nfwk.core:scoped_context;

import std.core;

export namespace nfwk {

class [[nodiscard]] scoped_logic {
public:

	scoped_logic() = default;

	template<typename Function>
	scoped_logic(Function&& clean) : clean{ std::move(clean) } {}

	scoped_logic(const scoped_logic&) = delete;

	scoped_logic(scoped_logic&& that) noexcept : clean{ std::move(that.clean) } {}

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

template<typename Resource>
class [[nodiscard]] scoped_context {
public:

	scoped_context() = default;

	scoped_context(std::optional<Resource> resource)
		: resource{ std::move(resource) } {}

	template<typename Function>
	scoped_context(Function&& clean)
		: scope{ std::move(clean) }, resource{ std::move(resource) } {}

	template<typename Function>
	scoped_context(Function&& clean, std::optional<Resource> resource)
		: scope{ std::move(clean) }, resource{ std::move(resource) } {}

	scoped_context(const scoped_context&) = delete;

	scoped_context(scoped_context&& that) noexcept
		: scope{ std::move(that.scope) }, resource{ std::move(that.resource) } {}

	scoped_context& operator=(const scoped_context&) = delete;

	scoped_context& operator=(scoped_context&& that) noexcept {
		std::swap(scope, that.scope);
		std::swap(resource, that.resource);
		return *this;
	}

	operator bool() const {
		return has_value();
	}

	bool has_value() const {
		return scope.operator bool() && resource.has_value();
	}

	const std::optional<Resource>& get() const {
		return resource;
	}

	std::optional<Resource>& get() {
		return resource;
	}

	operator const Resource& () const {
		return resource.value();
	}

private:

	scoped_logic scope;
	std::optional<Resource> resource;

};

}
