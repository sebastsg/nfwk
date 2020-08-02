#pragma once

#include "scripts/script_node_output.hpp"
#include "transform.hpp"

#include <optional>
#include <vector>
#include <functional>

namespace no {

class io_stream;

class script_node {
public:

	friend class script_tree;

	int id{ -1 };
	std::optional<int> scope_id;

	transform2 transform; // used in editor

	virtual int type() const = 0;
	virtual script_node_output_type output_type() const = 0;
	virtual std::string_view get_name() const = 0;
	virtual std::optional<int> process();
	virtual void write(io_stream& stream) const;
	virtual void read(io_stream& stream);
	virtual bool update_editor() = 0;
	virtual bool can_be_entry_point() const;
	virtual bool is_interactive() const;

	void delete_output_node(int node_id);
	void delete_output_slot(int slot);
	std::optional<int> get_output_node(int slot) const;
	std::optional<script_node_output> get_first_output() const;
	std::optional<int> get_first_output_node() const;
	void add_output(std::optional<int> slot, int to_node_id);
	bool has_interactive_output_nodes() const;

	const std::vector<script_node_output>& get_outputs() const;
	int used_output_slots_count() const;

protected:

	script_tree* tree{ nullptr };
	std::vector<script_node_output> outputs;

};

class script_node_constructor {
public:

	script_node_constructor(int type, std::string_view name, std::string_view category, const std::function<script_node*()>& constructor);
	script_node_constructor() = default;

	script_node* construct() const;
	std::optional<int> get_type() const;
	std::string_view get_name() const;
	std::string_view get_category() const;
	bool is_valid() const;

private:

	std::optional<int> type;
	std::string_view name;
	std::string_view category;
	std::function<script_node*()> constructor;

};

script_node* create_script_node(int type);
void register_script_node(int id, std::string_view name, std::string_view category, const std::function<script_node*()>& constructor);
const std::vector<script_node_constructor>& get_core_script_node_constructors();
const std::vector<script_node_constructor>& get_user_script_node_constructors();
std::vector<script_node_constructor> get_all_script_node_constructors();

template<typename T>
void register_script_node() {
	register_script_node(T::full_type, T::name, T::category, [] {
		return new T{};
	});
}

}

#define NFWK_SCRIPT_NODE_EXPLICIT(BASE, TYPE, NAME, CATEGORY) \
	static constexpr int full_type{ (BASE) + (TYPE) };\
	static constexpr int relative_type{ TYPE };\
	static constexpr std::string_view name{ NAME };\
	int type() const override { return full_type; }\
	std::string_view get_name() const override { return name; }\
	static constexpr std::string_view category{ CATEGORY }

#define NFWK_SCRIPT_CORE_NODE(TYPE, NAME, CATEGORY) NFWK_SCRIPT_NODE_EXPLICIT(0x0000, TYPE, NAME, CATEGORY)
#define NFWK_SCRIPT_USER_NODE(TYPE, NAME, CATEGORY) NFWK_SCRIPT_NODE_EXPLICIT(0xffff, TYPE, NAME, CATEGORY)
