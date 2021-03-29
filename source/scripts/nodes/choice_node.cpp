#include "scripts/nodes/choice_node.hpp"
#include "graphics/ui.hpp"
#include "io.hpp"

namespace nfwk {

void choice_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write_string(text);
}

void choice_node::read(io_stream& stream) {
	script_node::read(stream);
	text = stream.read_string();
}

bool choice_node::can_be_entry_point() const {
	return false;
}

bool choice_node::is_interactive() const {
	return true;
}

}
