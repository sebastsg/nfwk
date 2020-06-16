#include "noise.hpp"
#include "random.hpp"
#include "math.hpp"

namespace no {

// This code has been adapted for nfwk.
// Thanks to the original authors:
//  - Stefan Gustavson (stegu@itn.liu.se)
//  - Peter Eastman (peastman@drizzle.stanford.edu)

static constexpr vector3f gradients_3[12]{
	{ 1.0f, 1.0f, 0.0f }, { -1.0f,  1.0f, 0.0f }, { 1.0f, -1.0f,  0.0f }, { -1.0f, -1.0f,  0.0f },
	{ 1.0f, 0.0f, 1.0f }, { -1.0f,  0.0f, 1.0f }, { 1.0f,  0.0f, -1.0f }, { -1.0f,  0.0f, -1.0f },
	{ 0.0f, 1.0f, 1.0f }, {  0.0f, -1.0f, 1.0f }, { 0.0f,  1.0f, -1.0f }, {  0.0f, -1.0f, -1.0f }
};

static constexpr vector4f gradients_4[32]{
	{  0.0f,  1.0f, 1.0f, 1.0f }, {  0.0f,  1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f, -1.0f, 1.0f }, {  0.0f,  1.0f, -1.0f, -1.0f },
	{  0.0f, -1.0f, 1.0f, 1.0f }, {  0.0f, -1.0f,  1.0f, -1.0f }, {  0.0f, -1.0f, -1.0f, 1.0f }, {  0.0f, -1.0f, -1.0f, -1.0f },
	{  1.0f,  0.0f, 1.0f, 1.0f }, {  1.0f,  0.0f,  1.0f, -1.0f }, {  1.0f,  0.0f, -1.0f, 1.0f }, {  1.0f,  0.0f, -1.0f, -1.0f },
	{ -1.0f,  0.0f, 1.0f, 1.0f }, { -1.0f,  0.0f,  1.0f, -1.0f }, { -1.0f,  0.0f, -1.0f, 1.0f }, { -1.0f,  0.0f, -1.0f, -1.0f },
	{  1.0f,  1.0f, 0.0f, 1.0f }, {  1.0f,  1.0f,  0.0f, -1.0f }, {  1.0f, -1.0f,  0.0f, 1.0f }, {  1.0f, -1.0f,  0.0f, -1.0f },
	{ -1.0f,  1.0f, 0.0f, 1.0f }, { -1.0f,  1.0f,  0.0f, -1.0f }, { -1.0f, -1.0f,  0.0f, 1.0f }, { -1.0f, -1.0f,  0.0f, -1.0f },
	{  1.0f,  1.0f, 1.0f, 0.0f }, {  1.0f,  1.0f, -1.0f,  0.0f }, {  1.0f, -1.0f,  1.0f, 0.0f }, {  1.0f, -1.0f, -1.0f,  0.0f },
	{ -1.0f,  1.0f, 1.0f, 0.0f }, { -1.0f,  1.0f, -1.0f,  0.0f }, { -1.0f, -1.0f,  1.0f, 0.0f }, { -1.0f, -1.0f, -1.0f,  0.0f }
};

static constexpr vector4b simplex[64]{
	{ 0, 1, 2, 3 }, { 0, 1, 3, 2 }, { 0, 0, 0, 0 }, { 0, 2, 3, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 2, 3, 0 },
	{ 0, 2, 1, 3 }, { 0, 0, 0, 0 }, { 0, 3, 1, 2 }, { 0, 3, 2, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 1, 3, 2, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 1, 2, 0, 3 }, { 0, 0, 0, 0 }, { 1, 3, 0, 2 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 2, 3, 0, 1 }, { 2, 3, 1, 0 },
	{ 1, 0, 2, 3 }, { 1, 0, 3, 2 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 2, 0, 3, 1 }, { 0, 0, 0, 0 }, { 2, 1, 3, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 2, 0, 1, 3 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 3, 0, 1, 2 }, { 3, 0, 2, 1 }, { 0, 0, 0, 0 }, { 3, 1, 2, 0 },
	{ 2, 1, 0, 3 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 3, 1, 0, 2 }, { 0, 0, 0, 0 }, { 3, 2, 0, 1 }, { 3, 2, 1, 0 }
};

// todo: store this as 8 bits? the problem is the 256+ index access for the repeated list
static uint16_t permutation[512]{};
static uint16_t permutation_mod_12[512]{};

void set_noise_seed(unsigned long long seed) {
	random_number_generator rng{ seed };
	for (int i{ 0 }; i < 256; i++) {
		permutation[i] = rng.next(255);
		permutation_mod_12[i] = permutation[i] % 12;
	}
	for (int i{ 256 }; i < 512; i++) {
		permutation[i] = permutation[i - 256];
		permutation_mod_12[i] = permutation[i] % 12;
	}
}

float simplex_noise(float x, float y) {
	constexpr float F2{ 0.366025f }; // 0.5f * (std::sqrt(3.0f) - 1.0f);
	constexpr float G2{ 0.211325f }; // (3.0f - std::sqrt(3.0f)) / 6.0f;
	const float skew_factor{ (x + y) * F2 };
	const float i{ floor(x + skew_factor) };
	const float j{ floor(y + skew_factor) };
	const float unskew_factor{ (i + j) * G2 };
	const vector2f unskewed{ i - unskew_factor, j - unskew_factor };
	const vector2f origin_offset{ x - unskewed.x, y - unskewed.y };

	// for the 2D case, the simplex shape is an equilateral triangle.
	// offsets for second (middle) corner of simplex in (i, j) coords
	// lower triangle, XY order: (0, 0) -> (1, 0) -> (1, 1)
	// upper triangle, YX order: (0, 0) -> (0, 1) -> (1, 1)
	const auto order = origin_offset.x > origin_offset.y ? vector2i{ 1, 0 } : vector2i{ 0, 1 };

	// a step of (1,0) in (i,j) means a step of (1 - c,    -c) in (x, y), and
	// a step of (0,1) in (i,j) means a step of (   -c, 1 - c) in (x, y), where c = (3 - sqrt(3)) / 6
	const vector2f middle_offset{ origin_offset - order.to<float>() + G2 };
	const vector2f last_offset{ origin_offset - 1.0f + 2.0f * G2 };

	// work out the hashed gradient indices of the three simplex corners
	const int ii{ static_cast<int>(i) & 255 };
	const int jj{ static_cast<int>(j) & 255 };
	const auto gi0 = permutation_mod_12[ii + permutation[jj]];
	const auto gi1 = permutation_mod_12[ii + order.x + permutation[jj + order.y]];
	const auto gi2 = permutation_mod_12[ii + 1 + permutation[jj + 1]];
	float n{ 0.0f };
	if (float t{ 0.5f - origin_offset.x * origin_offset.x - origin_offset.y * origin_offset.y }; t >= 0.0f) {
		t *= t;
		n += t * t * gradients_3[gi0].xy.dot(origin_offset);
	}
	if (float t{ 0.5f - middle_offset.x * middle_offset.x - middle_offset.y * middle_offset.y }; t >= 0.0f) {
		t *= t;
		n += t * t * gradients_3[gi1].xy.dot(middle_offset);
	}
	if (float t{ 0.5f - last_offset.x * last_offset.x - last_offset.y * last_offset.y }; t >= 0.0f) {
		t *= t;
		n += t * t * gradients_3[gi2].xy.dot(last_offset);
	}
	return 70.0f * n;
}

float simplex_noise(float x, float y, float z) {
	constexpr float F3{ 1.0f / 3.0f };
	constexpr float G3{ 1.0f / 6.0f };
	const float skew_factor{ (x + y + z) * F3 };
	const float i{ floor(x + skew_factor) };
	const float j{ floor(y + skew_factor) };
	const float k{ floor(z + skew_factor) };
	const float unskew_factor{ (i + j + k) * G3 };
	const vector3f unskewed{ i - unskew_factor, j - unskew_factor, k - unskew_factor };
	const vector3f origin_offset{ x - unskewed.x, y - unskewed.y, z - unskewed.z };
	// for the 3D case, the simplex shape is a slightly irregular tetrahedron.
	vector3i order_2;
	vector3i order_3{ 1 };
	if (origin_offset.x >= origin_offset.y) {
		if (origin_offset.y >= origin_offset.z) {
			// X Y Z order
			order_2.x = 1;
			order_3.z = 0;
		} else if (origin_offset.x >= origin_offset.z) {
			// X Z Y order
			order_2.x = 1;
			order_3.y = 0;
		} else {
			// Z X Y order
			order_2.z = 1;
			order_3.y = 0;
		}
	} else {
		if (origin_offset.y < origin_offset.z) {
			// Z Y X order
			order_2.z = 1;
			order_3.x = 0;
		} else if (origin_offset.x < origin_offset.z) {
			// Y Z X order
			order_2.y = 1;
			order_3.x = 0;
		} else {
			// Y X Z order
			order_2.y = 1;
			order_3.z = 0;
		}
	}
	// a step of (1, 0, 0) in (i, j, k) means a step of (1 - c,    -c,    -c) in (x, y, z),
	// a step of (0, 1, 0) in (i, j, k) means a step of (   -c, 1 - c,    -c) in (x, y, z), and
	// a step of (0, 0, 1) in (i, j, k) means a step of (   -c,    -c, 1 - c) in (x, y, z), where c = 1/6.
	const vector3f offset_2{ origin_offset - order_2.to<float>() + G3 };
	const vector3f offset_3{ origin_offset - order_3.to<float>() + 2.0f * G3 };
	const vector3f offset_4{ origin_offset - 1.0f + 3.0f * G3 };
	const int ii{ static_cast<int>(i) & 255 };
	const int jj{ static_cast<int>(j) & 255 };
	const int kk{ static_cast<int>(k) & 255 };
	const auto gi0 = permutation_mod_12[ii + permutation[jj + permutation[kk]]];
	const auto gi1 = permutation_mod_12[ii + order_2.x + permutation[jj + order_2.y + permutation[kk + order_2.z]]];
	const auto gi2 = permutation_mod_12[ii + order_3.x + permutation[jj + order_3.y + permutation[kk + order_3.z]]];
	const auto gi3 = permutation_mod_12[ii + 1 + permutation[jj + 1 + permutation[kk + 1]]];
	float n{ 0.0f };
	if (float t{ 0.5f - origin_offset.x * origin_offset.x - origin_offset.y * origin_offset.y - origin_offset.z * origin_offset.z }; t >= 0) {
		t *= t;
		n += t * t * gradients_3[gi0].dot(origin_offset);
	}
	if (float t{ 0.5f - offset_2.x * offset_2.x - offset_2.y * offset_2.y - offset_2.z * offset_2.z }; t >= 0) {
		t *= t;
		n += t * t * gradients_3[gi1].dot(offset_2);
	}
	if (float t{ 0.5f - offset_3.x * offset_3.x - offset_3.y * offset_3.y - offset_3.z * offset_3.z }; t >= 0) {
		t *= t;
		n += t * t * gradients_3[gi2].dot(offset_3);
	}
	if (float t{ 0.5f - offset_4.x * offset_4.x - offset_4.y * offset_4.y - offset_4.z * offset_4.z }; t >= 0) {
		t *= t;
		n += t * t * gradients_3[gi3].dot(offset_4);
	}
	return 32.0f * n;
}

float simplex_noise(float x, float y, float z, float w) {
	constexpr float F4{ 0.309017f }; // (std::sqrt(5.0f) - 1.0f) / 4.0f;
	constexpr float G4{ 0.138197f }; // (5.0f - std::sqrt(5.0f)) / 20.0f;
	// Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
	const float skew_factor{ (x + y + z + w) * F4 };
	const float i{ floor(x + skew_factor) };
	const float j{ floor(y + skew_factor) };
	const float k{ floor(z + skew_factor) };
	const float l{ floor(w + skew_factor) };
	const float unskew_factor{ (i + j + k + l) * G4 };
	const vector4f unskewed{ i - unskew_factor, j - unskew_factor, k - unskew_factor, l - unskew_factor };
	const vector4f origin_offset{ x - unskewed.x, y - unskewed.y, z - unskewed.z, w - unskewed.w };

	// To find out which of the 24 possible simplices we're in, we need to determine the magnitude ordering of x0, y0, z0 and w0.
	// The method below is a good way to find the ordering of x,y,z,w and the correct traversal order for the simplex.
	// First, six pair-wise comparisons are performed between each possible pair
	// of the four coordinates, and the results are used to add up binary bits for an integer index.
	const int c1{ origin_offset.x > origin_offset.y ? 32 : 0 };
	const int c2{ origin_offset.x > origin_offset.z ? 16 : 0 };
	const int c3{ origin_offset.y > origin_offset.z ? 8 : 0 };
	const int c4{ origin_offset.x > origin_offset.w ? 4 : 0 };
	const int c5{ origin_offset.y > origin_offset.w ? 2 : 0 };
	const int c6{ origin_offset.z > origin_offset.w ? 1 : 0 };
	const auto& c = simplex[c1 + c2 + c3 + c4 + c5 + c6];
	// simplex[c] is a vector with the numbers 0, 1, 2 and 3 in some order.
	// Many values of c will never occur, since e.g. x>y>z>w makes x<z, y<w and x<w
	// impossible. Only the 24 indices which have non-zero entries make any sense.
	// We use a thresholding to set the coordinates in turn from the largest magnitude.
	const vector4i order_2{ c.x >= 3 ? 1 : 0, c.y >= 3 ? 1 : 0, c.z >= 3 ? 1 : 0, c.w >= 3 ? 1 : 0 };
	const vector4i order_3{ c.x >= 2 ? 1 : 0, c.y >= 2 ? 1 : 0, c.z >= 2 ? 1 : 0, c.w >= 2 ? 1 : 0 };
	const vector4i order_4{ c.x >= 1 ? 1 : 0, c.y >= 1 ? 1 : 0, c.z >= 1 ? 1 : 0, c.w >= 1 ? 1 : 0 };

	const vector4f offset_2{ origin_offset - order_2.to<float>() + G4 };
	const vector4f offset_3{ origin_offset - order_3.to<float>() + 2.0f * G4 };
	const vector4f offset_4{ origin_offset - order_4.to<float>() + 3.0f * G4 };
	const vector4f offset_5{ origin_offset - 1.0f + 4.0f * G4 };

	const int ii = static_cast<int>(i) & 255;
	const int jj = static_cast<int>(j) & 255;
	const int kk = static_cast<int>(k) & 255;
	const int ll = static_cast<int>(l) & 255;
	const auto gi0 = permutation[ii + permutation[jj + permutation[kk + permutation[ll]]]] % 32;
	const auto gi1 = permutation[ii + order_2.x + permutation[jj + order_2.y + permutation[kk + order_2.z + permutation[ll + order_2.w]]]] % 32;
	const auto gi2 = permutation[ii + order_3.x + permutation[jj + order_3.y + permutation[kk + order_3.z + permutation[ll + order_3.w]]]] % 32;
	const auto gi3 = permutation[ii + order_4.x + permutation[jj + order_4.y + permutation[kk + order_4.z + permutation[ll + order_4.w]]]] % 32;
	const auto gi4 = permutation[ii + 1 + permutation[jj + 1 + permutation[kk + 1 + permutation[ll + 1]]]] % 32;

	float n{ 0.0f };
	if (float t{ 0.5f - origin_offset.x * origin_offset.x - origin_offset.y * origin_offset.y - origin_offset.z * origin_offset.z - origin_offset.w * origin_offset.w }; t >= 0.0f) {
		t *= t;
		n += t * t * gradients_4[gi0].dot(origin_offset);
	}
	if (float t{ 0.5f - offset_2.x * offset_2.x - offset_2.y * offset_2.y - offset_2.z * offset_2.z - offset_2.w * offset_2.w }; t >= 0.0f) {
		t *= t;
		n += t * t * gradients_4[gi1].dot(offset_2);
	}
	if (float t{ 0.5f - offset_3.x * offset_3.x - offset_3.y * offset_3.y - offset_3.z * offset_3.z - offset_3.w * offset_3.w }; t >= 0.0f) {
		t *= t;
		n += t * t * gradients_4[gi2].dot(offset_3);
	}
	if (float t{ 0.5f - offset_4.x * offset_4.x - offset_4.y * offset_4.y - offset_4.z * offset_4.z - offset_4.w * offset_4.w }; t >= 0.0f) {
		t *= t;
		n += t * t * gradients_4[gi3].dot(offset_4);
	}
	if (float t{ 0.5f - offset_5.x * offset_5.x - offset_5.y * offset_5.y - offset_5.z * offset_5.z - offset_5.w * offset_5.w }; t >= 0.0f) {
		t *= t;
		n += t * t * gradients_4[gi4].dot(offset_5);
	}
	return 27.0f * n;
}

float simplex_octave_noise(int octaves, float persistence, float scale, float x, float y) {
	float total{ 0.0f };
	float frequency{ scale };
	float amplitude{ 1.0f };
	float max_amplitude{ 0.0f };
	for (int octave{ 0 }; octave < octaves; octave++) {
		total += simplex_noise(x * frequency, y * frequency) * amplitude;
		max_amplitude += amplitude;
		amplitude *= persistence;
		frequency *= 2.0f;
	}
	return total / max_amplitude;
}

}
