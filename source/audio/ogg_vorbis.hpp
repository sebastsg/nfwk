#pragma once

#include "audio/audio_source.hpp"
#include "io.hpp"

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

namespace no {

class ogg_vorbis_audio_source : public audio_source {
public:

	ogg_vorbis_audio_source(const std::filesystem::path& path);
	~ogg_vorbis_audio_source() override;

	size_t size() const override;
	pcm_format format() const override;
	const io_stream& stream() const override;
	int sample_rate() const override;

private:

	OggVorbis_File file;

	io_stream file_stream;
	io_stream pcm_stream;

	int frequency{ 0 };
	int channels{ 0 };

};

}
