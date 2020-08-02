#pragma once

#include "audio/pcm_stream.hpp"

namespace no {

class io_stream;

class audio_source {
public:

	virtual ~audio_source() = default;

	virtual size_t size() const = 0;
	virtual pcm_format format() const = 0;
	virtual const io_stream& stream() const = 0;
	virtual int sample_rate() const = 0;

};

}
