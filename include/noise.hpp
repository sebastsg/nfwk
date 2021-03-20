#pragma once

#include <cstdint>

namespace nfwk {

class simplex_noise_map {
public:

	const unsigned long long seed;

	simplex_noise_map(unsigned long long seed);

	float get(float x, float y) const;
	float get(float x, float y, float z) const;
	float get(float x, float y, float, float w) const;

	float get(int octaves, float persistence, float scale, float x, float y) const;

private:

	// todo: store this as 8 bits? the problem is the 256+ index access for the repeated list
	std::uint16_t permutation[512]{};
	std::uint16_t permutation_mod_12[512]{};

};

}
