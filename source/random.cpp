#include "random.hpp"
#include "log.hpp"

namespace nfwk {

random_number_generator::random_number_generator() {
	seed(std::random_device{}());
}

random_number_generator::random_number_generator(unsigned long long seed) {
	mersenne_twister_engine.seed(seed);
}

void random_number_generator::seed(unsigned long long seed) {
	current_seed = seed;
	mersenne_twister_engine.seed(seed);
}

unsigned long long random_number_generator::seed() const {
	return current_seed;
}

bool random_number_generator::chance(float chance) {
	return chance >= next<float>(0.0f, 1.0f);
}

std::string random_number_generator::string(int size) {
	std::string string;
	string.resize(size);
	std::generate_n(std::begin(string), size, [this] {
		constexpr std::string_view characters{ "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-_abcdefghijklmnopqrstuvwxyz" };
		return characters[next(characters.size() - 1)];
	});
	return string;
}

random_number_generator& random_number_generator::global() {
	static thread_local random_number_generator rng;
	static thread_local bool print_seed{ true };
	if (print_seed) {
		info("main", "Global random seed for this thread: {}", rng.seed());
		print_seed = false;
	}
	return rng;
}

}
