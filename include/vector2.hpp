#pragma once

#include <ostream>
#include <cstdint>

namespace nfwk {

template<typename T>
struct vector2 {

	T x{};
	T y{};

	constexpr vector2() = default;
	constexpr vector2(T i) : x{ i }, y{ i } {}
	constexpr vector2(T x, T y) : x{ x }, y{ y } {}

	constexpr vector2<T> operator-() const {
		return { -x, -y };
	}

	constexpr vector2<T> operator+(const vector2<T>& v) const {
		return { x + v.x, y + v.y };
	}

	constexpr vector2<T> operator-(const vector2<T>& v) const {
		return { x - v.x, y - v.y };
	}

	constexpr vector2<T> operator*(const vector2<T>& v) const {
		return { x * v.x, y * v.y };
	}

	constexpr vector2<T> operator/(const vector2<T>& v) const {
		return { x / v.x, y / v.y };
	}

	constexpr vector2<T> operator%(const vector2<T>& v) const {
		return { x % v.x, y % v.y };
	}

	constexpr vector2<T> operator+(T s) const {
		return { x + s, y + s };
	}

	constexpr vector2<T> operator-(T s) const {
		return { x - s, y - s };
	}

	constexpr vector2<T> operator*(T s) const {
		return { x * s, y * s };
	}

	constexpr vector2<T> operator/(T s) const {
		return { x / s, y / s };
	}

	constexpr vector2<T> operator%(T s) const {
		return { x % s, y % s };
	}

	void operator+=(const vector2<T>& v) {
		x += v.x;
		y += v.y;
	}

	void operator-=(const vector2<T>& v) {
		x -= v.x;
		y -= v.y;
	}

	void operator*=(const vector2<T>& v) {
		x *= v.x;
		y *= v.y;
	}

	void operator/=(const vector2<T>& v) {
		x /= v.x;
		y /= v.y;
	}

	void operator%=(const vector2<T>& v) {
		x %= v.x;
		y %= v.y;
	}

	constexpr bool operator>(const vector2<T>& v) const {
		return x > v.x && y > v.y;
	}

	constexpr bool operator<(const vector2<T>& v) const {
		return x < v.x&& y < v.y;
	}

	constexpr bool operator>=(const vector2<T>& v) const {
		return x >= v.x && y >= v.y;
	}

	constexpr bool operator<=(const vector2<T>& v) const {
		return x <= v.x && y <= v.y;
	}

	constexpr bool operator==(const vector2<T>& v) const {
		return x == v.x && y == v.y;
	}

	constexpr bool operator!=(const vector2<T>& v) const {
		return x != v.x || y != v.y;
	}

	constexpr T distance_to(const vector2<T>& v) const {
		const T dx{ x - v.x };
		const T dy{ y - v.y };
		return static_cast<T>(std::sqrt(static_cast<double>(dx * dx + dy * dy)));
	}

	void floor() {
		x = std::floor(x);
		y = std::floor(y);
	}

	void ceil() {
		x = std::ceil(x);
		y = std::ceil(y);
	}

	constexpr vector2<T> to_floor() const {
		return { std::floor(x), std::floor(y) };
	}

	constexpr vector2<T> to_ceil() const {
		return { std::ceil(x), std::ceil(y) };
	}

	void abs() {
		x = std::abs(x);
		y = std::abs(y);
	}

	constexpr vector2<T> to_abs() const {
		return{ std::abs(x), std::abs(y) };
	}

	template<typename U>
	constexpr vector2<U> to() const {
		return vector2<U>{ static_cast<U>(x), static_cast<U>(y) };
	}

	constexpr T magnitude() const {
		return static_cast<T>(std::sqrt(static_cast<double>(x * x + y * y)));
	}

	constexpr T squared_magnitude() const {
		return x * x + y * y;
	}

	constexpr vector2<T> normalized() const {
		if (const T m{ magnitude() }; m != T{}) {
			return { x / m, y / m };
		} else {
			return {};
		}
	}

	constexpr T dot(const vector2<T>& v) const {
		return x * v.x + y * v.y;
	}

	constexpr vector2<T> with_x(T s) const {
		return { s, y };
	}

	constexpr vector2<T> with_y(T s) const {
		return { x, s };
	}

};

using vector2b = vector2<int8_t>;
using vector2i = vector2<int32_t>;
using vector2l = vector2<int64_t>;
using vector2f = vector2<float>;
using vector2d = vector2<double>;

constexpr int reduce_index(vector2i index, int width) {
	return index.x * width + index.y;
}

constexpr int reduce_index(int x, int y, int width) {
	return x * width + y;
}

constexpr vector2i expand_index(int index, vector2i size) {
	return { index / size.x, index % size.y };
}

constexpr vector2i expand_index(int index, int width, int height) {
	return { index / width, index % height };
}

}

template<typename T>
nfwk::vector2<T> operator*(T scalar, const nfwk::vector2<T>& vector) {
	return vector * scalar;
}

template<typename T>
nfwk::vector2<T> operator/(T scalar, const nfwk::vector2<T>& vector) {
	return nfwk::vector2<T>{ scalar } / vector;
}
namespace std {

template<typename T>
ostream& operator<<(ostream& out, const nfwk::vector2<T>& vector) {
	return out << vector.x << ", " << vector.y;
}

template <typename T>
struct tuple_size<nfwk::vector2<T>> : integral_constant<size_t, 2> {};

template <size_t Index, typename T>
struct tuple_element<Index, const nfwk::vector2<T>> {
	static_assert(Index < 2, "Vector2 index is out of bounds");
	using type = T;
};

template <typename T>
constexpr T vector2_get(const nfwk::vector2<T>& vector, integral_constant<size_t, 0>) noexcept {
	return vector.x;
}

template <typename T>
constexpr T vector2_get(const nfwk::vector2<T>& vector, integral_constant<size_t, 1>) noexcept {
	return vector.y;
}

template <size_t Index, class T>
[[nodiscard]] constexpr tuple_element_t<Index, const nfwk::vector2<T>> get(const nfwk::vector2<T>& vector) noexcept {
	return vector2_get<T>(vector, integral_constant<size_t, Index>());
}

}
