module;

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

export module nfwk.audio:ogg_vorbis;

import std.core;
import std.filesystem;
import nfwk.core;
import :interfaces;

export namespace nfwk {

class ogg_vorbis_audio_source : public audio_source {
public:

	ogg_vorbis_audio_source(const std::filesystem::path& path) {
		read_file(path, file_stream);
		ov_callbacks callbacks{
			// read
			[](void* pointer, std::size_t object_size, std::size_t object_count, void* source) -> std::size_t {
				std::size_t size{ object_size * object_count };
				auto file_stream = static_cast<io_stream*>(source);
				if (const std::size_t remaining{ file_stream->size_left_to_read() }; size > remaining) {
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

		message("audio", "Loading ogg file: {}", path);
		if (const int result{ ov_open_callbacks(&file_stream, &file, nullptr, 0, callbacks) }; result != 0) {
			warning("audio", "The stream is invalid. Error: {}", result);
			return;
		}

		const auto* file_info = ov_info(&file, -1);
		channels = file_info->channels;
		frequency = file_info->rate;
		info("audio", "Ogg file info:\nChannels: {}\nFrequency: {}", channels, frequency);

		char buffer[8192];
		int bit_stream{ 0 };
		while (true) {
			if (const auto bytes = ov_read(&file, buffer, sizeof(buffer), 0, 2, 1, &bit_stream); bytes > 0) {
				pcm_stream.write(buffer, static_cast<std::size_t>(bytes));
			} else {
				break;
			}
		}
		ov_clear(&file);
	}

	~ogg_vorbis_audio_source() override {
		//ov_clear(&file);
	}

	std::size_t size() const override {
		return pcm_stream.size();
	}

	pcm_format format() const override {
		return pcm_format::int_16;
	}

	const io_stream& stream() const override {
		return pcm_stream;
	}

	int sample_rate() const override {
		return frequency;
	}

private:

	OggVorbis_File file;

	io_stream file_stream;
	io_stream pcm_stream;

	int channels{ 0 };
	int frequency{ 0 };

};

}
