#include "scripts/nodes/execute_script_node.hpp"
#include "scripts/script_tree.hpp"
#include "graphics/ui.hpp"
#include "io.hpp"

namespace nfwk::script {

std::optional<int> execute_script_node::process(script_context& context) const {
	ASSERT(false);
	error(scripts::log, "Not implemented");
	//script_tree inner;
	//inner.context = context;
	//inner.load(script);
	//inner.process_entry_point();
	return 0;
}

void execute_script_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write_string(script_id);
}

void execute_script_node::read(io_stream& stream) {
	script_node::read(stream);
	script_id = stream.read_string();
}

}
