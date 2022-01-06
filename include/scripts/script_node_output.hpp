#pragma once

#include "assert.hpp"
#include "log.hpp"

namespace nfwk::script {

enum class output_type { unknown, variable, single, boolean };

class node_output {
public:

	node_output(int to_node_id, int slot_index) : to_node_id{ to_node_id }, slot_index{ slot_index } {
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
