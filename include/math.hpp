#pragma once

#include <ostream>
#include <cstdint>

namespace no {

constexpr float pi_f{ 3.14159265359f };
constexpr double pi{ 3.14159265359 };

constexpr float rad_to_deg(float x) {
	return x * 57.295779513082320876f; // x * (180 / pi)
}

constexpr double rad_to_deg(double x) {
	return x * 57.295779513082320876; // x * (180 / pi)
}

constexpr float deg_to_rad(float x) {
	return x * 0.0174532925199432957f; // x * (pi / 180)
}

constexpr double deg_to_rad(double x) {
	return x * 0.0174532925199432957; // x * (pi / 180)
}

constexpr int mod(int a, int b) {
	return (a % b + b) % b;
}

constexpr int divide_leftwards(int a, int b) {
	return a / b - (a < 0 && a % b != 0);
}

template<typename T>
inline T clamp(T value, T min, T max) {
	if (value > max) {
		return max;
	} else if (value < min) {
		return min;
	}
	return value;
}

template<typename T>
inline T inverse_clamp(T value, T min, T max) {
	if (value > max) {
		return min;
	} else if (value < min) {
		return max;
	}
	return value;
}

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
		return x < v.x && y < v.y;
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

using vector2i = vector2<int32_t>;
using vector2l = vector2<int64_t>;
using vector2f = vector2<float>;
using vector2d = vector2<double>;

template<typename T>
struct vector3 {

	union {
		struct {
			T x, y, z;
		};
		struct {
			vector2<T> xy;
			T _placeholder_z;
		};
	};

	constexpr vector3() : x{}, y{}, z{} {}
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
		return x < v.x && y < v.y && z < v.z;
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

using vector3i = vector3<int32_t>;
using vector3l = vector3<int64_t>;
using vector3f = vector3<float>;
using vector3d = vector3<double>;

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
		return x < v.x && y < v.y && z < v.z && w < v.w;
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

using vector4i = vector4<int32_t>;
using vector4l = vector4<int64_t>;
using vector4f = vector4<float>;
using vector4d = vector4<double>;

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
no::vector2<T> operator*(T scalar, const no::vector2<T>& vector) {
	return vector * scalar;
}

template<typename T>
no::vector2<T> operator/(T scalar, const no::vector2<T>& vector) {
	return no::vector2<T>{ scalar } / vector;
}

template<typename T>
no::vector3<T> operator*(T scalar, const no::vector3<T>& vector) {
	return vector * scalar;
}

template<typename T>
no::vector3<T> operator/(T scalar, const no::vector3<T>& vector) {
	return no::vector3<T>{ scalar } / vector;
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

// vector2

template<typename T>
ostream& operator<<(ostream& out, const no::vector2<T>& vector) {
	return out << vector.x << ", " << vector.y;
}

template <typename T>
struct tuple_size<no::vector2<T>> : integral_constant<size_t, 2> {};

template <size_t Index, typename T>
struct tuple_element<Index, const no::vector2<T>> {
	static_assert(Index < 2, "Vector2 index is out of bounds");
	using type = T;
};

template <typename T>
constexpr T vector2_get(const no::vector2<T>& vector, integral_constant<size_t, 0>) noexcept {
	return vector.x;
}

template <typename T>
constexpr T vector2_get(const no::vector2<T>& vector, integral_constant<size_t, 1>) noexcept {
	return vector.y;
}

template <size_t Index, class T>
[[nodiscard]] constexpr tuple_element_t<Index, const no::vector2<T>> get(const no::vector2<T>& vector) noexcept {
	return vector2_get<T>(vector, integral_constant<size_t, Index>());
}

// vector3

template<typename T>
ostream& operator<<(ostream& out, const no::vector3<T>& vector) {
	return out << vector.x << ", " << vector.y << ", " << vector.z;
}

template <typename T>
struct tuple_size<no::vector3<T>> : integral_constant<size_t, 3> {};

template <size_t Index, typename T>
struct tuple_element<Index, const no::vector3<T>> {
	static_assert(Index < 3, "Vector3 index is out of bounds");
	using type = T;
};

template <typename T>
constexpr T vector3_get(const no::vector3<T>& vector, integral_constant<size_t, 0>) noexcept {
	return vector.x;
}

template <typename T>
constexpr T vector3_get(const no::vector3<T>& vector, integral_constant<size_t, 1>) noexcept {
	return vector.y;
}

template <typename T>
constexpr T vector3_get(const no::vector3<T>& vector, integral_constant<size_t, 2>) noexcept {
	return vector.z;
}

template <size_t Index, class T>
[[nodiscard]] constexpr tuple_element_t<Index, const no::vector3<T>> get(const no::vector3<T>& vector) noexcept {
	return vector3_get<T>(vector, integral_constant<size_t, Index>());
}

// vector4

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
