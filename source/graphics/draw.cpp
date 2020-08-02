#include "platform.hpp"
#include "graphics/draw.hpp"

#if ENABLE_GRAPHICS

std::ostream& operator<<(std::ostream& out, no::swap_interval interval) {
	switch (interval) {
	case no::swap_interval::late: return out << "Late";
	case no::swap_interval::immediate: return out << "Immediate";
	case no::swap_interval::sync: return out << "Sync";
	default: return out << "Unknown";
	}
}

#endif
