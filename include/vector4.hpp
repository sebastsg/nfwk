#pragma once

#include "vector3.hpp"

namespace no {

template<typename T>
struct vector4 {

	union {
		struct {
			T x, y, z, w;
		};
		struct {
			vector2<T> xy;
			T _placeholder_z_0;
			T _placeholder_w_1;
		};
		struct {
			vector3<T> xyz;
			T _placeholder_w_2;
		};
		struct {
			T _placeholder_x_0;
			T _placeholder_y_0;
			vector2<T> zw;
		};
	};

	constexpr vector4() : x{}, y{}, z{}, w{} {}
	constexpr vector4(T i) : x{ i }, y{ i }, z{ i }, w{ i } {}
	constexpr vector4(T x, T y, T z, T w) : x{ x }, y{ y }, z{ z }, w{ w } {}

	constexpr vector4<T> operator-() const {
		return { -x, -y, -z, -w };
	}

	constexpr vector4<T> operator+(const vector4<T>& v) const {
		return { x + v.x, y + v.y, z + v.z, w + v.w };
	}

	constexpr vector4<T> operator-(const vector4<T>& v) const {
		return { x - v.x, y - v.y, z - v.z, w - v.w };
	}

	constexpr vector4<T> operator*(const vector4<T>& v) const {
		return { x * v.x, y * v.y, z * v.z, w * v.w };
	}

	constexpr vector4<T> operator/(const vector4<T>& v) const {
		return { x / v.x, y / v.y, z / v.z, w / v.w };
	}

	constexpr vector4<T> operator%(const vector4<T>& v) const {
		return { x % v.x, y % v.y, z % v.z, w % v.w };
	}

	void operator+=(const vector4<T>& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
	}

	void operator-=(const vector4<T>& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
	}

	void operator*=(const vector4<T>& v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
	}

	void operator/=(const vector4<T>& v) {
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
	}

	void operator%=(const vector4<T>& v) {
		x %= v.x;
		y %= v.y;
		z %= v.z;
		w %= v.w;
	}

	constexpr bool operator>(const vector4<T>& v) const {
		return x > v.x && y > v.y && z > v.z && w > v.w;
	}

	constexpr bool operator<(const vector4<T>& v) const {
		return x < v.x&& y < v.y&& z < v.z&& w < v.w;
	}

	constexpr bool operator>=(const vector4<T>& v) const {
		return x >= v.x && y >= v.y && z >= v.z && w >= v.w;
	}

	constexpr bool operator<=(const vector4<T>& v) const {
		return x <= v.x && y <= v.y && z <= v.z && w <= v.w;
	}

	constexpr bool operator==(const vector4<T>& v) const {
		return x == v.x && y == v.y && z == v.z && w == v.w;
	}

	constexpr bool operator!=(const vector4<T>& v) const {
		return x != v.x || y != v.y || z != v.z || w != v.w;
	}

	constexpr T distance_to(const vector4<T>& v) const {
		const T dx{ x - v.x };
		const T dy{ y - v.y };
		const T dz{ z - v.z };
		const T dw{ w - v.w };
		return static_cast<T>(std::sqrt(static_cast<double>(dx * dx + dy * dy + dz * dz + dw * dw)));
	}

	void floor() {
		x = std::floor(x);
		y = std::floor(y);
		z = std::floor(z);
		w = std::floor(w);
	}

	void ceil() {
		x = std::ceil(x);
		y = std::ceil(y);
		z = std::ceil(z);
		w = std::ceil(w);
	}

	constexpr vector4<T> to_floor() const {
		return { std::floor(x), std::floor(y), std::floor(z), std::floor(w) };
	}

	constexpr vector4<T> to_ceil() const {
		return { std::ceil(x), std::ceil(y), std::ceil(z), std::ceil(w) };
	}

	void abs() {
		x = std::abs(x);
		y = std::abs(y);
		z = std::abs(z);
		w = std::abs(w);
	}

	constexpr vector4<T> to_abs() const {
		return{ std::abs(x), std::abs(y), std::abs(z), std::abs(w) };
	}

	template<typename U>
	constexpr vector4<U> to() const {
		return vector4<U>{ static_cast<U>(x), static_cast<U>(y), static_cast<U>(z), static_cast<U>(w) };
	}

	constexpr T magnitude() const {
		return static_cast<T>(std::sqrt(static_cast<double>(x * x + y * y + z * z + w * w)));
	}

	constexpr T squared_magnitude() const {
		return x * x + y * y + z * z + w * w;
	}

	constexpr vector4<T> normalized() const {
		if (const T m{ magnitude() }; m != T{}) {
			return { x / m, y / m, z / m, w / m };
		} else {
			return {};
		}
	}

	constexpr T dot(const vector4<T>& v) const {
		return x * v.x + y * v.y + z * v.z + w * v.w;
	}

	constexpr vector4<T> with_x(T s) const {
		return { s, y, z, w };
	}

	constexpr vector4<T> with_y(T s) const {
		return { x, s, z, w };
	}

	constexpr vector4<T> with_z(T s) const {
		return { x, y, s, w };
	}

	constexpr vector4<T> with_w(T s) const {
		return { x, y, z, s };
	}

};

using vector4b = vector4<int8_t>;
using vector4i = vector4<int32_t>;
using vector4l = vector4<int64_t>;
using vector4f = vector4<float>;
using vector4d = vector4<double>;

}

template<typename T>
no::vector4<T> operator*(T scalar, const no::vector4<T>& vector) {
	return vector * scalar;
}

template<typename T>
no::vector4<T> operator/(T scalar, const no::vector4<T>& vector) {
	return no::vector4<T>{ scalar } / vector;
}

namespace std {

template<typename T>
ostream& operator<<(ostream& out, const no::vector4<T>& vector) {
	return out << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w;
}

template <typename T>
struct tuple_size<no::vector4<T>> : integral_constant<size_t, 4> {};

template <size_t Index, typename T>
struct tuple_element<Index, const no::vector4<T>> {
	static_assert(Index < 4, "Vector4 index is out of bounds");
	using type = T;
};

template <typename T>
constexpr T vector4_get(const no::vector4<T>& vector, integral_constant<size_t, 0>) noexcept {
	return vector.x;
}

template <typename T>
constexpr T vector4_get(const no::vector4<T>& vector, integral_constant<size_t, 1>) noexcept {
	return vector.y;
}

template <typename T>
constexpr T vector4_get(const no::vector4<T>& vector, integral_constant<size_t, 2>) noexcept {
	return vector.z;
}

template <typename T>
constexpr T vector4_get(const no::vector4<T>& vector, integral_constant<size_t, 3>) noexcept {
	return vector.w;
}

template <size_t Index, class T>
[[nodiscard]] constexpr tuple_element_t<Index, const no::vector4<T>> get(const no::vector4<T>& vector) noexcept {
	return vector4_get<T>(vector, integral_constant<size_t, Index>());
}

}
