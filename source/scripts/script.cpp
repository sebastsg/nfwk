#include "scripts/script.hpp"
#include "scripts/script_editor.hpp"
#include "scripts/nodes/message_node.hpp"
#include "scripts/nodes/choice_node.hpp"
#include "scripts/nodes/compare_variable_node.hpp"
#include "scripts/nodes/modify_variable_node.hpp"
#include "scripts/nodes/create_variable_node.hpp"
#include "scripts/nodes/variable_exists_node.hpp"
#include "scripts/nodes/delete_variable_node.hpp"
#include "scripts/nodes/random_output_node.hpp"
#include "scripts/nodes/random_condition_node.hpp"
#include "scripts/nodes/execute_script_node.hpp"
#include "scripts/nodes/trigger_event_node.hpp"
#include "debug.hpp"

namespace no::internal {

void initialize_scripts() {
	register_script_node<message_node>();
	register_script_node<choice_node>();
	register_script_node<compare_variable_node>();
	register_script_node<modify_variable_node>();
	register_script_node<create_variable_node>();
	register_script_node<variable_exists_node>();
	register_script_node<delete_variable_node>();
	register_script_node<random_output_node>();
	register_script_node<random_condition_node>();
	register_script_node<execute_script_node>();
	register_script_node<trigger_event_node>();
	register_editor<script_editor>();
}

}
