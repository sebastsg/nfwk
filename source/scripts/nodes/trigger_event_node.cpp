#include "scripts/nodes/trigger_event_node.hpp"
#include "graphics/ui.hpp"
#include "io.hpp"

namespace no {

std::optional<int> trigger_event_node::process() {

	return 0;
}

void trigger_event_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write(event_name);
}

void trigger_event_node::read(io_stream& stream) {
	script_node::read(stream);
	event_name = stream.read<std::string>();
}

bool trigger_event_node::update_editor() {
	return ui::input("##event-name", event_name);
}

}
