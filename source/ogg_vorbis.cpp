#include "ogg_vorbis.hpp"
#include "debug.hpp"

namespace no {

ogg_vorbis_audio_source::ogg_vorbis_audio_source(const std::string& path) {
	file::read(path, file_stream);
	ov_callbacks callbacks{
		// read
		[](void* pointer, size_t object_size, size_t object_count, void* source) -> size_t {
			size_t size{ object_size * object_count };
			auto file_stream = static_cast<io_stream*>(source);
			if (const size_t remaining{ file_stream->size_left_to_read() }; size > remaining) {
				size = remaining;
			}
			file_stream->read(static_cast<char*>(pointer), size);
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

	MESSAGE_X("audio", "Loading ogg file: " << path);
	if (const int result{ ov_open_callbacks(&file_stream, &file, nullptr, 0, callbacks) }; result != 0) {
		WARNING_X("audio", "The stream is invalid. Error: " << result);
		return;
	}

	const auto* info = ov_info(&file, -1);
	channels = info->channels;
	frequency = info->rate;
	INFO_X("audio", "Ogg file info:" << "\nChannels: " << info->channels << "\nFrequency: " << info->rate);

	char buffer[8192];
	int bit_stream{ 0 };
	while (true) {
		if (const auto bytes = ov_read(&file, buffer, sizeof(buffer), 0, 2, 1, &bit_stream); bytes > 0) {
			pcm_stream.write(buffer, static_cast<size_t>(bytes));
		} else {
			break;
		}
	}
	ov_clear(&file);
}

ogg_vorbis_audio_source::~ogg_vorbis_audio_source() {
	//ov_clear(&file);
}

size_t ogg_vorbis_audio_source::size() const {
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
