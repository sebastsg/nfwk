#include "audio/ogg_vorbis.hpp"
#include "log.hpp"

namespace nfwk {

ogg_vorbis_audio_source::ogg_vorbis_audio_source(const std::filesystem::path& path) {
	read_file(path, file_stream);
	ov_callbacks callbacks{
		// read
		[](void* pointer, std::size_t object_size, std::size_t object_count, void* source) -> std::size_t {
			auto size = object_size * object_count;
			auto file_stream = static_cast<io_stream*>(source);
			if (const auto remaining = file_stream->size_left_to_read(); size > remaining) {
				size = remaining;
			}
			file_stream->read_raw(static_cast<char*>(pointer), size);
			return size / object_size;
		},
		// seek
		[](void* source, ogg_int64_t offset, int whence) -> int {
			return -1;
		},
		// close
		[](void* source) -> int {
			return 0;
		},
		// tell
		[](void* source) -> long {
			return -1;
		}
	};

	message(audio::log, u8"Loading ogg file: {}", path);
	if (const int result{ ov_open_callbacks(&file_stream, &file, nullptr, 0, callbacks) }; result != 0) {
		warning(audio::log, u8"The stream is invalid. Error: {}", result);
		return;
	}

	const auto* ogg_info = ov_info(&file, -1);
	channels = ogg_info->channels;
	frequency = ogg_info->rate;
	info(audio::log, u8"Ogg file info:\nChannels: {}\nFrequency: {}", ogg_info->channels, ogg_info->rate);

	char buffer[8192];
	int bit_stream{ 0 };
	while (true) {
		if (const auto bytes = ov_read(&file, buffer, sizeof(buffer), 0, 2, 1, &bit_stream); bytes > 0) {
			pcm_stream.write_raw(buffer, static_cast<std::size_t>(bytes));
		} else {
			break;
		}
	}
	ov_clear(&file);
}

ogg_vorbis_audio_source::~ogg_vorbis_audio_source() {
	//ov_clear(&file);
}

std::size_t ogg_vorbis_audio_source::size() const {
	return pcm_stream.size();
}

pcm_format ogg_vorbis_audio_source::format() const {
	return pcm_format::int_16;
}

const io_stream& ogg_vorbis_audio_source::stream() const {
	return pcm_stream;
}

int ogg_vorbis_audio_source::sample_rate() const {
	return frequency;
}

}
