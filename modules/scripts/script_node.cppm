module;

#include "assert.hpp"

export module nfwk.scripts:node;

import std.core;
import nfwk.core;
export import :node_output;

export namespace nfwk {

class script_node {
public:

	friend class script_tree;

	int id{ -1 };
	std::optional<int> scope_id;

	transform2 transform; // used in editor

	script_node() = default;
	script_node(const script_node&) = delete;
	script_node(script_node&&) = default;

	virtual ~script_node() = default;

	script_node& operator=(const script_node&) = delete;
	script_node& operator=(script_node&&) = default;

	virtual int type() const = 0;
	virtual script_node_output_type output_type() const = 0;
	virtual std::string_view get_name() const = 0;

	virtual std::optional<int> process() const {
		return std::nullopt;
	}

	virtual void write(io_stream& stream) const {
		stream.write<std::int32_t>(id);
		stream.write_optional<std::int32_t>(scope_id);
		stream.write(transform);
		stream.write(static_cast<std::int32_t>(outputs.size()));
		for (const auto& output : outputs) {
			stream.write<std::int32_t>(output.to_node());
			stream.write<std::int32_t>(output.slot());
		}
	}

	virtual void read(io_stream& stream) {
		id = stream.read<std::int32_t>();
		scope_id = stream.read_optional<std::int32_t>();
		transform = stream.read<transform2>();
		const auto out_count = stream.read<std::int32_t>();
		for (int j{ 0 }; j < out_count; j++) {
			const auto node_id = stream.read<std::int32_t>();
			const auto out_id = stream.read<std::int32_t>();
			outputs.emplace_back(node_id, out_id);
		}
	}

	virtual bool can_be_entry_point() const {
		return true;
	}

	virtual bool is_interactive() const {
		return false;
	}
	
	[[nodiscard]] bool process_output(int slot) const {
		return false; // return tree->process_output(id, slot);
	}

	[[nodiscard]] bool process_outputs() const {
		return false; // tree->process_outputs(id);
	}

	void delete_output_node(int node_id) {
		std::erase_if(outputs, [node_id](const auto& output) {
			return output.to_node() == node_id;
		});
	}

	void delete_output_slot(int slot) {
		std::erase_if(outputs, [slot](const auto& output) {
			return output.slot() == slot;
		});
	}

	void add_output(std::optional<int> slot, int to_node_id) {
		if (!slot.has_value()) {
			slot = 0;
			while (get_output_node(slot.value())) {
				slot.value()++;
			}
		}
		for (auto& output : outputs) {
			if (output.slot() == slot.value()) {
				output.set_to_node(to_node_id);
				return;
			}
		}
		outputs.emplace_back(to_node_id, slot.value());
	}

	const std::vector<script_node_output>& get_outputs() const {
		return outputs;
	}

	std::optional<script_node_output> get_first_output() const {
		return outputs.empty() ? std::optional<script_node_output>{} : outputs[0];
	}

	std::optional<int> get_output_node(int slot) const {
		for (const auto& output : outputs) {
			if (output.slot() == slot) {
				return output.to_node();
			}
		}
		return std::nullopt;
	}

	std::optional<int> get_first_output_node() const {
		if (const auto output = get_first_output()) {
			return output->to_node();
		} else {
			return std::nullopt;
		}
	}

	int used_output_slots_count() const {
		return static_cast<int>(outputs.size());
	}

protected:

	script_tree* tree{ nullptr };
	std::vector<script_node_output> outputs;

};

}

namespace nfwk {

class script_node;
class script_tree;

export class script_node_constructor {
public:

	script_node_constructor(int type, std::string_view name, std::string_view category, const std::function<script_node* ()>& constructor)
		: type{ type }, name{ name }, category{ category }, constructor{ constructor } {}

	script_node_constructor() = default;

	script_node* construct() const {
		return constructor ? constructor() : nullptr;
	}

	std::optional<int> get_type() const {
		return type;
	}

	std::string_view get_name() const {
		return name;
	}

	std::string_view get_category() const {
		return category;
	}

	bool is_valid() const {
		return type.has_value() && constructor;
	}

private:

	std::optional<int> type;
	std::string_view name;
	std::string_view category;
	std::function<script_node* ()> constructor;

};

class script_node_factory {
public:

	const script_node_constructor* find_script_node_constructor(int type) const {
		if (type >= 0xffff) {
			type -= 0xffff;
			if (static_cast<int>(user_nodes.size()) > type) {
				ASSERT(user_nodes[type].get_type() == type);
				return &user_nodes[type];
			}
		} else {
			if (static_cast<int>(core_nodes.size()) > type) {
				ASSERT(core_nodes[type].get_type() == type);
				return &core_nodes[type];
			}
		}
		return nullptr;
	}

	script_node* create_script_node(int type) const {
		return find_script_node_constructor(type)->construct();
	}

	void register_script_node(int type, std::string_view name, std::string_view category, const std::function<script_node* ()>& constructor) {
		ASSERT(!find_script_node_constructor(type));
		if (type >= 0xffff) {
			type -= 0xffff;
			user_nodes.resize(type + 1);
			user_nodes[type] = { type, name, category, constructor };
		} else {
			core_nodes.resize(type + 1);
			core_nodes[type] = { type, name, category, constructor };
		}
	}

	const std::vector<script_node_constructor>& get_core_constructors() const {
		return core_nodes;
	}

	const std::vector<script_node_constructor>& get_user_constructors() const {
		return user_nodes;
	}

	std::vector<int> get_unused_script_node_type_gaps() const {
		std::vector<int> unused;
		for (int type{ 0 }; type < static_cast<int>(core_nodes.size()); type++) {
			if (!core_nodes[type].is_valid()) {
				unused.push_back(type);
			}
		}
		for (int type{ 0 }; type < static_cast<int>(user_nodes.size()); type++) {
			if (!user_nodes[type].is_valid()) {
				unused.push_back(type);
			}
		}
		return unused;
	}

private:

	std::vector<script_node_constructor> core_nodes;
	std::vector<script_node_constructor> user_nodes;

};

script_node_factory node_factory;

}

export namespace nfwk {

script_node* create_script_node(int type) {
	return node_factory.create_script_node(type);
}

void register_script_node(int id, std::string_view name, std::string_view category, const std::function<script_node*()>& constructor) {
	node_factory.register_script_node(id, name, category, constructor);
}

const std::vector<script_node_constructor>& get_core_script_node_constructors() {
	return node_factory.get_core_constructors();
}

const std::vector<script_node_constructor>& get_user_script_node_constructors() {
	return node_factory.get_user_constructors();
}

std::vector<script_node_constructor> get_all_script_node_constructors() {
	auto constructors = merge_vectors(node_factory.get_core_constructors(), node_factory.get_user_constructors());
	std::erase_if(constructors, [](const auto& constructor) {
		return !constructor.is_valid();
	});
	return constructors;
}

std::vector<int> get_unused_script_node_type_gaps() {
	return node_factory.get_unused_script_node_type_gaps();
}

template<typename T>
void register_script_node() {
	register_script_node(T::full_type, T::name, T::category, [] {
		return new T{};
	});
}

}
