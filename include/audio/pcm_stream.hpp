#pragma once

#include <cstdint>
#include <ostream>

namespace no {

class audio_source;

enum class pcm_format { float_32, int_16, unknown };

class pcm_stream {
public:

	pcm_stream(audio_source* source);

	bool is_empty() const;
	float read_float();
	void reset();
	void stream(pcm_format format, uint8_t* destination, size_t size, size_t channels);
	void stream(float* destination, size_t count, size_t channels);
	int sample_rate() const;

private:

	size_t position{ 0 };
	audio_source* source{ nullptr };

};

}

std::ostream& operator<<(std::ostream& out, no::pcm_format format);
