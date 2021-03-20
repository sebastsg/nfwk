#pragma once

namespace nfwk {

class audio_player;

class audio_endpoint {
public:

	virtual ~audio_endpoint() = default;

	virtual audio_player* add_player() = 0;
	virtual void stop_all_players() = 0;
	virtual void clear_players() = 0;

};

}
