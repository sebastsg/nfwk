#pragma once

namespace no {

void set_noise_seed(unsigned long long seed);

float octave_noise(float octaves, float persistence, float scale, float x, float y);
float octave_noise(float octaves, float persistence, float scale, float x, float y, float z);
float octave_noise(float octaves, float persistence, float scale, float x, float y, float z, float w);

float scaled_octave_noise(float octaves, float persistence, float scale, float loBound, float hiBound, float x, float y);
float scaled_octave_noise(float octaves, float persistence, float scale, float loBound, float hiBound, float x, float y, float z);
float scaled_octave_noise(float octaves, float persistence, float scale, float loBound, float hiBound, float x, float y, float z, float w);

float scaled_raw_noise(float lower, float higher, float x, float y);
float scaled_raw_noise(float lower, float higher, float x, float y, float z);
float scaled_raw_noise(float lower, float higher, float x, float y, float z, float w);

float raw_noise(float x, float y);
float raw_noise(float x, float y, float z);
float raw_noise(float x, float y, float, float w);

}
