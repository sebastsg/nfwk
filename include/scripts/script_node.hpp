#pragma once

#include "scripts/script_node_output.hpp"
#include "transform.hpp"

#include <optional>
#include <vector>
#include <functional>

namespace nfwk {

class io_stream;
class script_tree;

class script_node {
public:

	friend class script_tree;

	int id{ -1 };
	std::optional<int> scope_id;

	transform2 transform; // used in editor

	script_node() = default;
	script_node(const script_node&) = delete;
	script_node(script_node&&) = delete;
	
	virtual ~script_node() = default;

	script_node& operator=(const script_node&) = delete;
	script_node& operator=(script_node&&) = delete;

	virtual int type() const = 0;
	virtual script_node_output_type output_type() const = 0;
	virtual std::u8string_view get_name() const = 0;
	
	virtual std::optional<int> process() const;
	virtual void write(io_stream& stream) const;
	virtual void read(io_stream& stream);
	virtual bool can_be_entry_point() const;
	virtual bool is_interactive() const;

	void delete_output_node(int node_id);
	void delete_output_slot(int slot);
	std::optional<int> get_output_node(int slot) const;
	std::optional<script_node_output> get_first_output() const;
	std::optional<int> get_first_output_node() const;
	void add_output(std::optional<int> slot, int to_node_id);

#if 0
	bool has_interactive_output_nodes() const;
#endif
	
	const std::vector<script_node_output>& get_outputs() const;
	int used_output_slots_count() const;

	script_tree& get_tree() const {
		return *tree;
	}

protected:

	script_tree* tree{ nullptr };
	std::vector<script_node_output> outputs;

};

class script_node_constructor {
public:

	script_node_constructor(int type, std::u8string_view name, std::u8string_view category, const std::function<std::shared_ptr<script_node>()>& constructor);
	script_node_constructor() = default;

	std::shared_ptr<script_node> construct() const;
	std::optional<int> get_type() const;
	std::u8string_view get_name() const;
	std::u8string_view get_category() const;
	bool is_valid() const;

private:

	std::optional<int> type;
	std::u8string_view name;
	std::u8string_view category;
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
	
	void register_node(int type, std::u8string_view name, std::u8string_view category, const std::function<std::shared_ptr<script_node>()>& constructor);

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
