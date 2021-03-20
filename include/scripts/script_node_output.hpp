#pragma once

namespace nfwk {

enum class script_node_output_type { unknown, variable, single, boolean };

class script_node_output {
public:

	script_node_output(int to_node_id, int slot_index);

	int slot() const;
	int to_node() const;
	void set_to_node(int to_node);

private:

	int to_node_id{ -1 };
	int slot_index{ 0 };

};

}
