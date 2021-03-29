#include "scripts/script_node_output.hpp"
#include "log.hpp"
#include "assert.hpp"

namespace nfwk {

script_node_output::script_node_output(int to_node_id, int slot_index) : to_node_id{ to_node_id }, slot_index{ slot_index } {
	ASSERT(to_node_id >= 0);
}

int script_node_output::slot() const {
	return slot_index;
}

int script_node_output::to_node() const {
	return to_node_id;
}

void script_node_output::set_to_node(int to_node) {
	to_node_id = to_node;
}

}
