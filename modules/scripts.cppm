export module nfwk.scripts;

export import :variables;
export import :node;
//export import :tree;

/*export import :node.message;
export import :node.choice;
export import :node.compare_variable;
export import :node.modify_variable;
export import :node.create_variable;
export import :node.variable_exists;
export import :node.delete_variable;
export import :node.random_output;
export import :node.random_condition;
export import :node.execute_script;
export import :node.trigger_event;
export import :node.spawn_object;*/

export import :object_instance;
export import :object_class;
export import :object_renderer;
export import :objects;
export import :events;
export import :script_utility_functions;

export namespace nfwk::internal {

void initialize_scripts() {
	/*register_script_node<message_node>();
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
	register_script_node<spawn_object_node>();*/
}

}
