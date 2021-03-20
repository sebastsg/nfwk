#include "network/network.hpp"

std::ostream& operator<<(std::ostream& out, nfwk::socket_close_status status) {
	switch (status) {
	case nfwk::socket_close_status::disconnected_gracefully: return out << "Disconnected gracefully";
	case nfwk::socket_close_status::connection_reset: return out << "Connection reset";
	case nfwk::socket_close_status::not_connected: return out << "Not connected";
	case nfwk::socket_close_status::unknown: return out << "Unknown";
	default: return out << "Invalid (" << static_cast<int>(status) << ")";
	}
}
