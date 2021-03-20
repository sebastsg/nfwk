#pragma once

#include <random>

namespace nfwk {

class random_number_generator {
public:

	random_number_generator();
	random_number_generator(unsigned long long seed);

	void seed(unsigned long long seed);
	unsigned long long seed() const;

	// min and max are inclusive
	template<typename T>
	T next(T min, T max) {
		if constexpr (std::is_integral<T>::value) {
			std::uniform_int_distribution<T> distribution{ min, max };
			return distribution(mersenne_twister_engine);
		} else if constexpr (std::is_floating_point<T>::value) {
			std::uniform_real_distribution<T> distribution{ min, max };
			return distribution(mersenne_twister_engine);
		}
		static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "T is not an integral or floating point type");
	}

	// max is inclusive
	template<typename T>
	T next(T max) {
		return next<T>(static_cast<T>(0), max);
	}

	// chance must be between 0.0f and 1.0f
	// the higher chance is, the more likely this function returns true
	bool chance(float chance);

	std::string string(int size);

	static random_number_generator& global();

private:

	std::mt19937_64 mersenne_twister_engine;
	unsigned long long current_seed{ 0 };

};

}
