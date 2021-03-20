#include "platform.hpp"
#include "graphics/draw.hpp"
#include "graphics/window.hpp"

std::ostream& operator<<(std::ostream& out, nfwk::swap_interval interval) {
	switch (interval) {
	case nfwk::swap_interval::late: return out << "Late";
	case nfwk::swap_interval::immediate: return out << "Immediate";
	case nfwk::swap_interval::sync: return out << "Sync";
	default: return out << "Unknown";
	}
}
