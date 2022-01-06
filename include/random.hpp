#pragma once

#include <random>

namespace nfwk {

class random_number_generator {
public:

	random_number_generator(unsigned long long seed = std::random_device{}());

	void reseed(unsigned long long seed);
	unsigned long long seed() const;

	template<typename T>
	T next(T min_inclusive, T max_inclusive) {
		static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "T must be integral or floating point");
		if constexpr (std::is_integral<T>::value) {
			return std::uniform_int_distribution<T>{ min_inclusive, max_inclusive }(mersenne_twister_engine);
		} else if constexpr (std::is_floating_point<T>::value) {
			return std::uniform_real_distribution<T>{ min_inclusive, max_inclusive }(mersenne_twister_engine);
		}
	}

	template<typename T>
	T next(T max_inclusive) {
		return next<T>({}, max_inclusive);
	}

	// chance must be between 0.0f and 1.0f
	// the higher chance is, the more likely this function returns true
	bool chance(float chance);

	std::string characters(int size, std::string_view reference_characters);
	std::string string(int size);

	static random_number_generator& any();

private:

	std::mt19937_64 mersenne_twister_engine;
	unsigned long long current_seed{ 0 };

};

}
