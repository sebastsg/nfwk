#include "scripts/nodes/execute_script_node.hpp"
#include "scripts/script_tree.hpp"
#include "graphics/ui.hpp"
#include "io.hpp"

namespace nfwk {

std::optional<int> execute_script_node::process() {
	script_tree inner;
	inner.context = tree->context;
	ASSERT(false);
	error("scripts", "Not implemented");
	//inner.load(script);
	//inner.process_entry_point();
	return 0;
}

void execute_script_node::write(io_stream& stream) const {
	script_node::write(stream);
	stream.write(script);
}

void execute_script_node::read(io_stream& stream) {
	script_node::read(stream);
	script = stream.read<std::string>();
}

bool execute_script_node::update_editor() {
	bool dirty = ui::input("Script", script);
	return dirty;
}

}
