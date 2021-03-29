#include "scripts/nodes/message_node.hpp"
#include "graphics/ui.hpp"
#include "io.hpp"

namespace nfwk {

void message_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write_string(text);
}

void message_node::read(io_stream& stream) {
	script_node::read(stream);
	text = stream.read_string();
}

bool message_node::is_interactive() const {
	return true;
}

}
