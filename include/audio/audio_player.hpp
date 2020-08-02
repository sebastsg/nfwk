#pragma once

#include "audio/pcm_stream.hpp"

namespace no {

class pcm_stream;
class audio_source;

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

}
