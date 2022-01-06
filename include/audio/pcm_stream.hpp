#pragma once

#include <cstdint>

#include "io.hpp"

namespace nfwk {

class audio_source;

enum class pcm_format { float_32, int_16, unknown };

class pcm_stream {
public:

	pcm_stream(audio_source* source);

	bool is_empty() const;
	float read_float();
	void reset();
	void stream(pcm_format format, std::uint8_t* destination, std::size_t size, std::size_t channels);
	void stream(float* destination, std::size_t count, std::size_t channels);
	int sample_rate() const;

private:

	std::size_t position{ 0 };
	audio_source* source{ nullptr };

};

}

std::ostream& operator<<(std::ostream& out, nfwk::pcm_format value);

NFWK_STDSPEC_FORMATTER(nfwk::pcm_format);

