export module nfwk.scripts:object_instance;

import std.core;
import nfwk.core;

export namespace nfwk::objects {

class object_instance {
public:

	transform2 transform;

	object_instance(int class_index) : class_index{ static_cast<std::uint16_t>(class_index) } {}

	object_instance(const object_instance&) = delete;

	object_instance(object_instance&& that) noexcept {
		std::swap(transform, that.transform);
		std::swap(flags, that.flags);
		std::swap(reserved, that.reserved);
		std::swap(class_index, that.class_index);
	}

	~object_instance() = default;

	object_instance& operator=(const object_instance&) = delete;

	object_instance& operator=(object_instance&& that) noexcept {
		std::swap(transform, that.transform);
		std::swap(flags, that.flags);
		std::swap(reserved, that.reserved);
		std::swap(class_index, that.class_index);
		return *this;
	}

	void kill() {
		flags |= flag::dead;
	}

	bool is_alive() const {
		return !(flags & flag::dead);
	}

	void make_persistent() {
		flags |= flag::persistent;
	}

	bool is_persistent() const {
		return !(flags & flag::persistent);
	}
	
	void write(io_stream& stream) const {
		stream.write(transform);
	}

	void read(io_stream& stream) {
		transform = stream.read<transform2>();
	}

private:

	struct flag {
		static constexpr std::uint8_t dead{ 0b00000001 };
		static constexpr std::uint8_t persistent{ 0b00000010 };
		static constexpr std::uint8_t res2{ 0b00000100 };
		static constexpr std::uint8_t res3{ 0b00001000 };
		static constexpr std::uint8_t res4{ 0b00010000 };
		static constexpr std::uint8_t res5{ 0b00100000 };
		static constexpr std::uint8_t res6{ 0b01000000 };
		static constexpr std::uint8_t res7{ 0b10000000 };
	};

	std::uint8_t flags{ 0 };
	std::uint8_t reserved{ 0 };
	std::uint16_t class_index{ 0 };

};

}
