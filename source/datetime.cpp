#include "datetime.hpp"
#include "io.hpp"

namespace nfwk {

std::string format_time(std::string_view format, std::time_t time) {
	char buffer[64]{};
	tm time_data;
	localtime_s(&time_data, &time); // todo: not portable atm
	std::strftime(buffer, sizeof(buffer), format.data(), &time_data);
	return buffer;
}

}
