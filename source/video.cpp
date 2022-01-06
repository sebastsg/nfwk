#include "video.hpp"
#include "log.hpp"
#include "graphics/color.hpp"
#include "math.hpp"

namespace nfwk {

video_player::video_player(const std::filesystem::path& path) {
	if (path.extension().u8string() != u8".webm") {
		return;
	}
	const auto path_string = path_to_string(path);
	if (mkv_reader.Open(path_string.c_str())) {
		warning("graphics", "Failed to load video: {}", path_string);
		return;
	}
	long long offset{ 0 };
	mkvparser::EBMLHeader ebml_header;
	ebml_header.Parse(&mkv_reader, offset);
	if (mkvparser::Segment::CreateInstance(&mkv_reader, offset, segment) < 0) {
		warning("graphics", "Failed to create segment.");
		return;
	}
	if (segment->Load() < 0) {
		warning("graphics", "Failed to load segment.");
		return;
	}
	const auto* segment_info = segment->GetInfo();
	if (!segment_info) {
		warning("graphics", "Failed to get segment info.");
		return;
	}
	time_code_scale = segment_info->GetTimeCodeScale();
	duration_ns = segment_info->GetDuration();
	if (const auto* utf8_title = segment_info->GetTitleAsUTF8()) {
		title = reinterpret_cast<const char*>(utf8_title);
	}
	const auto* tracks = segment->GetTracks();
	for (unsigned int track_index{ 0 }; track_index < tracks->GetTracksCount(); track_index++) {
		const auto* track = tracks->GetTrackByIndex(track_index);
		if (!track) {
			warning("graphics", "Skipped unknown track {}", track_index);
			continue;
		}
		if (track->GetType() == mkvparser::Track::kVideo) {
			if (const auto* video_track = dynamic_cast<const mkvparser::VideoTrack*>(track)) {
				const std::string codec_id{ reinterpret_cast<const char*>(video_track->GetCodecId()) };
				if (codec_id == "V_VP8") {
					vpx_interface = &vpx_codec_vp8_dx_algo;
				} else if (codec_id == "V_VP9") {
					vpx_interface = &vpx_codec_vp9_dx_algo;
				} else {
					warning("graphics", "Video codec not supported: {}", codec_id);
					return;
				}
				frame_rate = static_cast<long long>(video_track->GetDefaultDuration());
				size = { static_cast<int>(video_track->GetWidth()), static_cast<int>(video_track->GetHeight()) };
				vpx_config.w = static_cast<unsigned int>(size.x);
				vpx_config.h = static_cast<unsigned int>(size.y);
				vpx_config.threads = 1;
				vpx_codec_flags_t flags{};
				if (vpx_codec_dec_init(&vpx_context, vpx_interface, &vpx_config, flags)) {
					warning("graphics", "Failed to initialize decoder.");
					return;
				}
			}
		} else if (track->GetType() == mkvparser::Track::kAudio) {
			if (const auto* audio_track = dynamic_cast<const mkvparser::AudioTrack*>(track)) {
				
			}
		}
	}
}

video_player::~video_player() {
	stop();
	vpx_codec_destroy(&vpx_context);
}

void video_player::play() {
	stop();
	if (!segment) {
		return;
	}
	decode_thread = std::thread{ [this] {
		play_timer.start();
		cluster = segment->GetFirst();
		stop_decoding = false;
		while (!stop_decoding) {
			if (paused) {
				std::this_thread::yield();
				continue;
			}
			if (!decode()) {
				stop_decoding = true;
			}
		}
	} };
}

void video_player::stop() {
	play_timer.stop();
	stop_decoding = true;
	paused = false;
	if (decode_thread.joinable()) {
		decode_thread.join();
	}
}

void video_player::pause() {
	play_timer.pause();
	paused = true;
}

void video_player::resume() {
	play_timer.resume();
	paused = false;
}

bool video_player::decode() {
	if (block_entry && block_entry->EOS()) {
		return false;
	}
	if (!block_entry && cluster && !cluster->EOS()) {
		if (cluster->GetFirst(block_entry) < 0) {
			warning("graphics", "Failed to get cluster block");
			return false;
		}
	}

	//
	static float y_lookup[256]{};
	static float r_v_lookup[256]{};
	static float g_v_lookup[256]{};
	static float g_u_lookup[256]{};
	static float b_u_lookup[256]{};
	static bool lookup_initialized{ false };
	if (!lookup_initialized) {
		lookup_initialized = true;
		for (int i{ 0 }; i < 256; i++) {
			y_lookup[i] = 1.164f * static_cast<float>(i - 16);
		}
		for (int i{ 0 }; i < 256; i++) {
			r_v_lookup[i] = 1.596f * static_cast<float>(i - 128);
		}
		for (int i{ 0 }; i < 256; i++) {
			g_v_lookup[i] = 0.813f * static_cast<float>(i - 128);
		}
		for (int i{ 0 }; i < 256; i++) {
			g_u_lookup[i] = 0.391f * static_cast<float>(i - 128);
		}
		for (int i{ 0 }; i < 256; i++) {
			b_u_lookup[i] = 2.018f * static_cast<float>(i - 128);
		}
	}
	//
	
	while (cluster && block_entry) {
		const auto* block = block_entry->GetBlock();
		const auto track_number = static_cast<long>(block->GetTrackNumber());
		const auto* track = segment->GetTracks()->GetTrackByNumber(track_number);
		if (!track) {
			warning("graphics", "Did not find track {}", track_number);
			return false;
		}
		bool got_frame{ false };
		if (track->GetType() == mkvparser::Track::Type::kVideo) {
			for (int frame_index{ 0 }; frame_index < block->GetFrameCount(); frame_index++) {
				const auto& frame = block->GetFrame(frame_index);
				io_stream frame_stream{ static_cast<std::size_t>(frame.len) };
				if (mkv_reader.Read(frame.pos, frame.len, reinterpret_cast<unsigned char*>(frame_stream.at_write())) < 0) {
					warning("graphics", "Failed to read frame buffer.");
					return false;
				}
				frame_stream.set_write_index(frame.len);
				const auto* stream_read = reinterpret_cast<unsigned char*>(frame_stream.at_read());
				vpx_codec_stream_info_t stream_info{};
				stream_info.sz = sizeof(stream_info);
				vpx_codec_peek_stream_info(vpx_interface, stream_read, frame_stream.size_left_to_read(), &stream_info);
				if (vpx_codec_decode(&vpx_context, stream_read, frame_stream.size_left_to_read(), nullptr, 0) != VPX_CODEC_OK) {
					warning("graphics", "Failed to decode video frame.");
					return false;
				}
				
				vpx_codec_iter_t it{ nullptr };
				vpx_image* image{ nullptr };
				while ((image = vpx_codec_get_frame(&vpx_context, &it))) {
					if (image->fmt != VPX_IMG_FMT_I420) {
						warning("graphics", "Image format not supported: {}", static_cast<int>(image->fmt));
						return false;
					}
					got_frame = true;

					auto new_frame = std::make_unique<video_frame>(size, block->GetTime(cluster));

					const auto y_stride = image->stride[VPX_PLANE_Y];
					const auto u_stride = image->stride[VPX_PLANE_U];
					const auto v_stride = image->stride[VPX_PLANE_V];

					const auto y_plane = image->planes[VPX_PLANE_Y];
					const auto u_plane = image->planes[VPX_PLANE_U];
					const auto v_plane = image->planes[VPX_PLANE_V];

					int y_offset{ 0 };
					int u_offset{ 0 };
					int v_offset{ 0 };
					const auto width = static_cast<int>(image->d_w);
					const auto height = static_cast<int>(image->d_h);
					for (int column{ 0 }; column < width; column++) {
						auto y_index = y_offset;
						auto u_index = u_offset;
						auto v_index = v_offset;
						for (int row{ 0 }; row < height; row++) {
							const auto y = y_plane[y_index];
							const auto u = u_plane[u_index];
							const auto v = v_plane[v_index];
							const float r = clamp(y_lookup[y] + r_v_lookup[v], 0.0f, 255.0f);
							const float g = clamp(y_lookup[y] - g_v_lookup[v] - g_u_lookup[u], 0.0f, 255.0f);
							const float b = clamp(y_lookup[y] + b_u_lookup[u], 0.0f, 255.0f);
							new_frame->frame_surface.set(column, row, f255_to_rgba(vector3f{ r, g, b }));
							y_index += y_stride;
							if ((row & 1) == 0) {
								u_index += u_stride;
								v_index += v_stride;
							}
						}
						y_offset++;
						if ((column & 1) == 0) {
							u_offset++;
							v_offset++;
						}
					}
					std::lock_guard lock{ sync_mutex };
					frames.emplace_back(std::move(new_frame));
				}
			}
		}
		if (cluster->GetNext(block_entry, block_entry) < 0) {
			warning("graphics", "Failed to get next block in cluster");
			return false;
		}
		if (!block_entry || block_entry->EOS()) {
			cluster = segment->GetNext(cluster);
		}
		if (got_frame) {
			return true;
		}
	}
	return false;
}

void video_player::refresh(texture& frame_texture) {
	std::lock_guard lock{ sync_mutex };
	if (frames.empty()) {
		if (stop_decoding) {
			pause();
		}
		return;
	}
	if (auto& frame = frames.front(); frame && play_timer.nanoseconds() - frame->time_ns >= frame_rate) {
		frame_texture.load(frame->frame_surface, scale_option::linear, false);
		frames.erase(frames.begin());
	}
}

vector2i video_player::get_size() const {
	return size;
}

float video_player::get_aspect_ratio() const {
	return static_cast<float>(size.x) / static_cast<float>(size.y);
}

float video_player::completion() const {
	if (!play_timer.has_started()) {
		return 0.0f;
	}
	return static_cast<float>(static_cast<double>(play_timer.nanoseconds()) / static_cast<double>(duration_ns));
}


}
