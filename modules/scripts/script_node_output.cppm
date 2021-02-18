module;

#include "assert.hpp"

export module nfwk.scripts:node_output;

import nfwk.core;

export namespace nfwk {

enum class script_node_output_type { unknown, variable, single, boolean };

class script_node_output {
public:

	script_node_output(int to_node_id, int slot_index) : to_node_id{ to_node_id }, slot_index{ slot_index } {
		ASSERT(to_node_id >= 0);
	}

	int slot() const {
		return slot_index;
	}
	
	int to_node() const {
		return to_node_id;
	}

	void set_to_node(int to_node) {
		to_node_id = to_node;
	}

private:

	int to_node_id{ -1 };
	int slot_index{ 0 };

};

}
