export module nfwk.network:core;

import std.core;
import nfwk.core;

export namespace nfwk {

enum class socket_close_status { disconnected_gracefully, connection_reset, not_connected, unknown };

struct socket_events {
	event<io_stream> stream;
	event<io_stream> packet;
	event<socket_close_status> disconnect;
	event<int> accept;
};

}

#if 0
export std::ostream& operator<<(std::ostream& out, nfwk::socket_close_status status) {
	switch (status) {
	case nfwk::socket_close_status::disconnected_gracefully: return out << "Disconnected gracefully";
	case nfwk::socket_close_status::connection_reset: return out << "Connection reset";
	case nfwk::socket_close_status::not_connected: return out << "Not connected";
	case nfwk::socket_close_status::unknown: return out << "Unknown";
	default: return out << "Invalid (" << static_cast<int>(status) << ")";
	}
}
#endif
