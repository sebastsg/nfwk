#pragma once

#include "audio/pcm_stream.hpp"

namespace nfwk {

class io_stream;

class audio_source {
public:

	audio_source() = default;
	audio_source(const audio_source&) = delete;
	audio_source(audio_source&&) = delete;

	virtual ~audio_source() = default;

	audio_source& operator=(const audio_source&) = delete;
	audio_source& operator=(audio_source&&) = delete;

	virtual std::size_t size() const = 0;
	virtual pcm_format format() const = 0;
	virtual const io_stream& stream() const = 0;
	virtual int sample_rate() const = 0;

};

}
