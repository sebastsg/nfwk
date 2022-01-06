#pragma once

#include "timer.hpp"
#include "graphics/surface.hpp"
#include "io.hpp"
#include "graphics/texture.hpp"

#include "libwebm/mkvreader.hpp"
#include "libwebm/mkvparser.hpp"
#include "vpx/vpx_codec.h"
#include "vpx/vpx_decoder.h"
#include "vpx/vp8.h"
#include "vpx/vp8dx.h"
#include "vpx/vp8cx.h"

#include <filesystem>
#include <mutex>
#include <thread>

namespace nfwk {

class video_player {
public:

	class video_frame {
	public:

		surface frame_surface;
		long long time_ns{ 0 };

		video_frame(vector2i size, long long time_ns)
			: frame_surface{ size.x, size.y, pixel_format::rgba, 0xff000000 }, time_ns{ time_ns } {}
	};
	
	std::vector<std::unique_ptr<video_frame>> frames;

	video_player(const std::filesystem::path& path);
	video_player(const video_player&) = delete;
	video_player(video_player&&) = delete;

	~video_player();

	video_player& operator=(const video_player&) = delete;
	video_player& operator=(video_player&&) = delete;

	long long get_duration_in_seconds() {
		return duration_ns / 1000000000;
	}
	
	std::string get_title() const {
		return title;
	}

	void play();
	void pause();
	void resume();
	void stop();
	
	void refresh(texture& frame_texture);

	vector2i get_size() const;
	float get_aspect_ratio() const;

	float completion() const;

private:
	
	bool decode();

	long long time_code_scale{ 0 };
	long long duration_ns{ 0 };
	std::string title;
	timer play_timer;
	mkvparser::Segment* segment{ nullptr };
	vpx_codec_dec_cfg_t vpx_config{};
	vpx_codec_iface_t* vpx_interface{ nullptr };
	mkvparser::MkvReader mkv_reader;
	vpx_codec_ctx_t vpx_context{};
	long long frame_rate{ 0 };
	vector2i size;
	const mkvparser::BlockEntry* block_entry{ nullptr };
	const mkvparser::Cluster* cluster{ nullptr };
	std::thread decode_thread;
	std::mutex sync_mutex;
	std::atomic<bool> stop_decoding{ false };
	std::atomic<bool> paused{ false };
	
};

}

//const float r = clamp(1.164f * (y - 16.0f) + 1.596f * (v - 128.0f), 0.0f, 255.0f);
//const float g = clamp(1.164f * (y - 16.0f) - 0.813f * (v - 128.0f) - 0.391f * (u - 128.0f), 0.0f, 255.0f);
//const float b = clamp(1.164f * (y - 16.0f) + 2.018f * (u - 128.0f), 0.0f, 255.0f);