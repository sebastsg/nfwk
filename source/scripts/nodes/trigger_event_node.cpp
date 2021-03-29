#include "scripts/nodes/trigger_event_node.hpp"
#include "graphics/ui.hpp"
#include "io.hpp"

namespace nfwk {

std::optional<int> trigger_event_node::process() const {
	warning("scripts", "Does not pause script yet. Should be made like execute_script_node.");
	//game_event_container::global().trigger(event_id);
	return 0;
}

void trigger_event_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write_string(event_id);
}

void trigger_event_node::read(io_stream& stream) {
	script_node::read(stream);
	event_id = stream.read_string();
}

}
