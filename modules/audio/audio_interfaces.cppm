module;

#include "assert.hpp"

export module nfwk.audio:interfaces;

import std.core;
import nfwk.core;

export namespace nfwk {

enum class pcm_format { float_32, int_16, unknown };

class audio_source {
public:

	virtual ~audio_source() = default;

	virtual std::size_t size() const = 0;
	virtual pcm_format format() const = 0;
	virtual const io_stream& stream() const = 0;
	virtual int sample_rate() const = 0;

};

class pcm_stream {
public:

	pcm_stream(audio_source* source) : source{ source } {}

	bool is_empty() const {
		switch (source->format()) {
		case pcm_format::int_16: return position + sizeof(std::int16_t) >= source->size();
		case pcm_format::float_32: return position + sizeof(float) >= source->size();
		default: return true;
		}
	}

	float read_float() {
		if (is_empty()) {
			return 0.0f;
		}
		if (source->format() == pcm_format::int_16) {
			const auto pcm = source->stream().read<std::int16_t>(position);
			position += sizeof(pcm);
			return static_cast<float>(static_cast<double>(pcm) / static_cast<double>(std::numeric_limits<std::int16_t>::max()));
		}
		warning("audio", "Unknown source format {}", source->format());
		return 0.0f;
	}

	void reset() {
		position = 0;
	}

	void stream(pcm_format format, std::uint8_t* destination, std::size_t size, std::size_t channels) {
		switch (format) {
		case pcm_format::float_32:
			stream(reinterpret_cast<float*>(destination), size / sizeof(float), channels);
			break;
		default:
			ASSERT(false);
			break;
		}
	}

	void stream(float* destination, std::size_t count, std::size_t channels) {
		for (std::size_t i{ 0 }; i < count; i += channels) {
			for (std::size_t j{ 0 }; j < channels; j++) {
				destination[i + j] = read_float();
			}
		}
	}

	int sample_rate() const {
		return source->sample_rate();
	}

private:

	std::size_t position{ 0 };
	audio_source* source{ nullptr };

};

class audio_player {
public:

	virtual ~audio_player() = default;

	virtual void play(const pcm_stream& stream) = 0;
	virtual void play(audio_source* source) = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual void stop() = 0;
	virtual void loop() = 0;
	virtual void once() = 0;
	virtual void set_volume(float volume) = 0;

	virtual bool is_playing() const = 0;
	virtual bool is_paused() const = 0;
	virtual bool is_looping() const = 0;

};

class audio_endpoint {
public:

	virtual ~audio_endpoint() = default;

	virtual audio_player* add_player() = 0;
	virtual void stop_all_players() = 0;
	virtual void clear_players() = 0;

};

}

#if 0
export std::ostream& operator<<(std::ostream& out, nfwk::pcm_format format) {
	switch (format) {
	case nfwk::pcm_format::float_32: return out << "Float32";
	case nfwk::pcm_format::int_16: return out << "Int16";
	default: return out << "Unknown";
	}
}
#endif
