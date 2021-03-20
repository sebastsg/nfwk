#include "audio/pcm_stream.hpp"
#include "audio/audio_source.hpp"
#include "log.hpp"

#include <climits>

namespace nfwk {

pcm_stream::pcm_stream(audio_source* source) : source{ source } {

}

bool pcm_stream::is_empty() const {
	switch (source->format()) {
	case pcm_format::int_16: return position + sizeof(std::int16_t) >= source->size();
	case pcm_format::float_32: return position + sizeof(float) >= source->size();
	default: return true;
	}
}

float pcm_stream::read_float() {
	if (is_empty()) {
		return 0.0f;
	}
	if (source->format() == pcm_format::int_16) {
		const auto pcm = source->stream().read<std::int16_t>(position);
		position += sizeof(pcm);
		return static_cast<float>(static_cast<double>(pcm) / static_cast<double>(SHRT_MAX));
	}
	warning("audio", "Unknown source format {}", source->format());
	return 0.0f;
}

void pcm_stream::reset() {
	position = 0;
}

void pcm_stream::stream(pcm_format format, std::uint8_t* destination, std::size_t size, std::size_t channels) {
	switch (format) {
	case pcm_format::float_32:
		stream(reinterpret_cast<float*>(destination), size / sizeof(float), channels);
		break;
	default:
		ASSERT(false);
		break;
	}
}

void pcm_stream::stream(float* destination, std::size_t count, std::size_t channels) {
	for (std::size_t i{ 0 }; i < count; i += channels) {
		for (std::size_t j{ 0 }; j < channels; j++) {
			destination[i + j] = read_float();
		}
	}
}

int pcm_stream::sample_rate() const {
	return source->sample_rate();
}

}

std::ostream& operator<<(std::ostream& out, nfwk::pcm_format format) {
	switch (format) {
	case nfwk::pcm_format::float_32: return out << "Float32";
	case nfwk::pcm_format::int_16: return out << "Int16";
	default: return out << "Unknown";
	}
}
