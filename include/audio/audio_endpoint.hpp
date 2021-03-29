#pragma once

namespace nfwk {

class audio_player;

class audio_endpoint {
public:

	audio_endpoint() = default;
	audio_endpoint(const audio_endpoint&) = delete;
	audio_endpoint(audio_endpoint&&) = delete;
	
	virtual ~audio_endpoint() = default;

	audio_endpoint& operator=(const audio_endpoint&) = delete;
	audio_endpoint& operator=(audio_endpoint&&) = delete;

	virtual audio_player* add_player() = 0;
	virtual void stop_all_players() = 0;
	virtual void clear_players() = 0;

};

}
