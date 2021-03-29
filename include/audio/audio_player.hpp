#pragma once

#include "audio/pcm_stream.hpp"

namespace nfwk {

class pcm_stream;
class audio_source;

class audio_player {
public:

	audio_player() = default;
	audio_player(const audio_player&) = delete;
	audio_player(audio_player&&) = delete;
	
	virtual ~audio_player() = default;

	audio_player& operator=(const audio_player&) = delete;
	audio_player& operator=(audio_player&&) = delete;

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

}
