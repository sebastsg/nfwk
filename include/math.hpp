#pragma once

namespace nfwk {

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

constexpr float floor(float x) {
	return static_cast<float>(static_cast<int>(x) - (x < 0.0f));
}

template<typename T>
constexpr T clamp(T value, T min, T max) {
	if (value > max) {
		return max;
	} else if (value < min) {
		return min;
	} else {
		return value;
	}
}

template<typename T>
constexpr T inverse_clamp(T value, T min, T max) {
	if (value > max) {
		return min;
	} else if (value < min) {
		return max;
	} else {
		return value;
	}
}

}
