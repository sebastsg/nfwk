#include "network/network.hpp"

#if ENABLE_NETWORK

std::ostream& operator<<(std::ostream& out, no::socket_close_status status) {
	switch (status) {
	case no::socket_close_status::disconnected_gracefully: return out << "Disconnected gracefully";
	case no::socket_close_status::connection_reset: return out << "Connection reset";
	case no::socket_close_status::not_connected: return out << "Not connected";
	case no::socket_close_status::unknown: return out << "Unknown";
	default: return out << "Invalid (" << static_cast<int>(status) << ")";
	}
}

#endif
