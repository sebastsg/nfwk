#pragma once

#include "scripts/script_node_output.hpp"
#include "transform.hpp"

#include <optional>
#include <vector>
#include <functional>

namespace nfwk {
class io_stream;
}

namespace nfwk::script {

class script_context;

class script_node {
public:

	friend class script_tree;

	transform2 transform; // used in editor

	script_node() = default;
	script_node(const script_node&) = delete;
	script_node(script_node&&) = delete;
	
	virtual ~script_node() = default;

	script_node& operator=(const script_node&) = delete;
	script_node& operator=(script_node&&) = delete;

	virtual int type() const = 0;
	virtual output_type get_output_type() const = 0;
	virtual std::string_view get_name() const = 0;
	
	virtual std::optional<int> process(script_context& context) const {
		return std::nullopt;
	}
	
	virtual void write(io_stream& stream) const;
	virtual void read(io_stream& stream);
	virtual bool can_be_entry_point() const;
	virtual bool is_interactive() const;

	void delete_output_node(int node_id);
	void delete_output_slot(int slot);
	std::optional<int> get_output_node(int slot) const;
	std::optional<node_output> get_first_output() const;
	std::optional<int> get_first_output_node() const;
	void add_output(std::optional<int> slot, int to_node_id);
	
	const std::vector<node_output>& get_outputs() const;
	int used_output_slots_count() const;

	void set_id(int new_id) {
		id = new_id;
	}

	std::optional<int> get_id() const {
		return id;
	}

protected:

	std::vector<node_output> outputs;

private:

	std::optional<int> id;

};

class script_node_constructor {
public:

	script_node_constructor(int type, std::string_view name, std::string_view category, std::function<std::shared_ptr<script_node>()> constructor);
	script_node_constructor() = default;

	std::shared_ptr<script_node> construct() const;
	std::optional<int> get_type() const;
	std::string_view get_name() const;
	std::string_view get_category() const;
	bool is_valid() const;

private:

	std::optional<int> type;
	std::string_view name;
	std::string_view category;
	std::function<std::shared_ptr<script_node>()> constructor;

};

class script_node_factory {
public:

	script_node_factory();

	const script_node_constructor* find_constructor(int type) const;
	std::shared_ptr<script_node> create_node(int type) const;

	const std::vector<script_node_constructor>& get_core_constructors() const;
	const std::vector<script_node_constructor>& get_user_constructors() const;
	std::vector<script_node_constructor> get_all_constructors() const;
	
	void register_node(int type, std::string_view name, std::string_view category, const std::function<std::shared_ptr<script_node>()>& constructor);

	template<typename T>
	void register_node() {
		register_node(T::full_type, T::name, T::category, [] {
			return std::make_shared<T>();
		});
	}

private:

	std::vector<script_node_constructor> core_nodes;
	std::vector<script_node_constructor> user_nodes;

};

}
