#pragma once

#include "vector2.hpp"

namespace nfwk {

template<typename T>
struct vector3 {

	static constexpr int components{ 3 };

	T x{};
	T y{};
	T z{};

	constexpr vector2<T> xy() const {
		return { x, y };
	}

	constexpr vector3() = default;
	constexpr vector3(T i) : x{ i }, y{ i }, z{ i } {}
	constexpr vector3(T x, T y, T z) : x{ x }, y{ y }, z{ z } {}
	
	constexpr vector3<T> operator-() const {
		return { -x, -y, -z };
	}

	constexpr vector3<T> operator+(const vector3<T>& v) const {
		return { x + v.x, y + v.y, z + v.z };
	}

	constexpr vector3<T> operator-(const vector3<T>& v) const {
		return { x - v.x, y - v.y, z - v.z };
	}

	constexpr vector3<T> operator*(const vector3<T>& v) const {
		return { x * v.x, y * v.y, z * v.z };
	}

	constexpr vector3<T> operator/(const vector3<T>& v) const {
		return { x / v.x, y / v.y, z / v.z };
	}

	constexpr vector3<T> operator%(const vector3<T>& v) const {
		return { x % v.x, y % v.y, z % v.z };
	}

	void operator+=(const vector3<T>& v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}

	void operator-=(const vector3<T>& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}

	void operator*=(const vector3<T>& v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
	}

	void operator/=(const vector3<T>& v) {
		x /= v.x;
		y /= v.y;
		z /= v.z;
	}

	void operator%=(const vector3<T>& v) {
		x %= v.x;
		y %= v.y;
		z %= v.z;
	}

	constexpr bool operator>(const vector3<T>& v) const {
		return x > v.x && y > v.y && z > v.z;
	}

	constexpr bool operator<(const vector3<T>& v) const {
		return x < v.x&& y < v.y&& z < v.z;
	}

	constexpr bool operator>=(const vector3<T>& v) const {
		return x >= v.x && y >= v.y && z >= v.z;
	}

	constexpr bool operator<=(const vector3<T>& v) const {
		return x <= v.x && y <= v.y && z <= v.z;
	}

	constexpr bool operator==(const vector3<T>& v) const {
		return x == v.x && y == v.y && z == v.z;
	}

	constexpr bool operator!=(const vector3<T>& v) const {
		return x != v.x || y != v.y || z != v.z;
	}

	constexpr T distance_to(const vector3<T>& v) const {
		const T dx{ x - v.x };
		const T dy{ y - v.y };
		const T dz{ z - v.z };
		return static_cast<T>(std::sqrt(static_cast<double>(dx * dx + dy * dy + dz * dz)));
	}

	void floor() {
		x = std::floor(x);
		y = std::floor(y);
		z = std::floor(z);
	}

	void ceil() {
		x = std::ceil(x);
		y = std::ceil(y);
		z = std::ceil(z);
	}

	constexpr vector3<T> to_floor() const {
		return { std::floor(x), std::floor(y), std::floor(z) };
	}

	constexpr vector3<T> to_ceil() const {
		return { std::ceil(x), std::ceil(y), std::ceil(z) };
	}

	void abs() {
		x = std::abs(x);
		y = std::abs(y);
		z = std::abs(z);
	}

	constexpr vector3<T> to_abs() const {
		return{ std::abs(x), std::abs(y), std::abs(z) };
	}

	template<typename U>
	constexpr vector3<U> to() const {
		return vector3<U>{ static_cast<U>(x), static_cast<U>(y), static_cast<U>(z) };
	}

	constexpr T magnitude() const {
		return static_cast<T>(std::sqrt(static_cast<double>(x * x + y * y + z * z)));
	}

	constexpr T squared_magnitude() const {
		return x * x + y * y + z * z;
	}

	constexpr vector3<T> normalized() const {
		if (const T m{ magnitude() }; m != T{}) {
			return { x / m, y / m, z / m };
		} else {
			return {};
		}
	}

	constexpr T dot(const vector3<T>& v) const {
		return x * v.x + y * v.y + z * v.z;
	}

	constexpr vector3<T> cross(const vector3<T>& v) const {
		return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x };
	}

	constexpr vector3<T> with_x(T s) const {
		return { s, y, z };
	}

	constexpr vector3<T> with_y(T s) const {
		return { x, s, z };
	}

	constexpr vector3<T> with_z(T s) const {
		return { x, y, s };
	}

};

using vector3b = vector3<int8_t>;
using vector3i = vector3<int32_t>;
using vector3l = vector3<int64_t>;
using vector3f = vector3<float>;
using vector3d = vector3<double>;

}

template<typename T>
nfwk::vector3<T> operator*(T scalar, const nfwk::vector3<T>& vector) {
	return vector * scalar;
}

template<typename T>
nfwk::vector3<T> operator/(T scalar, const nfwk::vector3<T>& vector) {
	return nfwk::vector3<T>{ scalar } / vector;
}

namespace std {

template<typename T>
ostream& operator<<(ostream& out, const nfwk::vector3<T>& vector) {
	return out << vector.x << ", " << vector.y << ", " << vector.z;
}

template <typename T>
struct tuple_size<nfwk::vector3<T>> : integral_constant<size_t, 3> {};

template <size_t Index, typename T>
struct tuple_element<Index, const nfwk::vector3<T>> {
	static_assert(Index < 3, "Vector3 index is out of bounds");
	using type = T;
};

template <typename T>
constexpr T vector3_get(const nfwk::vector3<T>& vector, integral_constant<size_t, 0>) noexcept {
	return vector.x;
}

template <typename T>
constexpr T vector3_get(const nfwk::vector3<T>& vector, integral_constant<size_t, 1>) noexcept {
	return vector.y;
}

template <typename T>
constexpr T vector3_get(const nfwk::vector3<T>& vector, integral_constant<size_t, 2>) noexcept {
	return vector.z;
}

template <size_t Index, class T>
[[nodiscard]] constexpr tuple_element_t<Index, const nfwk::vector3<T>> get(const nfwk::vector3<T>& vector) noexcept {
	return vector3_get<T>(vector, integral_constant<size_t, Index>());
}

}
