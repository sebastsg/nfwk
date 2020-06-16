#pragma once

namespace no {

void set_noise_seed(unsigned long long seed);

float simplex_noise(float x, float y);
float simplex_noise(float x, float y, float z);
float simplex_noise(float x, float y, float, float w);

float simplex_octave_noise(int octaves, float persistence, float scale, float x, float y);

}
