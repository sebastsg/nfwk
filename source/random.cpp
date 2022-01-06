#include "random.hpp"
#include "log.hpp"

namespace nfwk {

random_number_generator::random_number_generator(unsigned long long seed)
	: mersenne_twister_engine{ seed }, current_seed{ seed } {}

void random_number_generator::reseed(unsigned long long seed) {
	mersenne_twister_engine.seed(seed);
	current_seed = seed;
}

unsigned long long random_number_generator::seed() const {
	return current_seed;
}

bool random_number_generator::chance(float chance) {
	return chance >= next<float>(0.0f, 1.0f);
}

std::string random_number_generator::characters(int size, std::string_view reference_characters) {
	std::string string;
	string.resize(size);
	std::generate_n(std::begin(string), size, [this, reference_characters] {
		return reference_characters[next(reference_characters.size() - 1)];
	});
	return string;
}

std::string random_number_generator::string(int size) {
	return characters(size, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-_abcdefghijklmnopqrstuvwxyz");
}

random_number_generator& random_number_generator::any() {
	thread_local random_number_generator rng;
	thread_local bool print_seed{ true };
	if (print_seed) {
		info(core::log, "Random seed for this thread: {}", rng.seed());
		print_seed = false;
	}
	return rng;
}

}
