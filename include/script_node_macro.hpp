#pragma once

#define NFWK_SCRIPT_NODE_EXPLICIT(BASE, TYPE, NAME, CATEGORY) \
	static constexpr int full_type{ (BASE) + (TYPE) };\
	static constexpr int relative_type{ TYPE };\
	static constexpr std::string_view name{ NAME };\
	int type() const override { return full_type; }\
	std::string_view get_name() const override { return name; }\
	static constexpr std::string_view category{ CATEGORY }

#define NFWK_SCRIPT_CORE_NODE(TYPE, NAME, CATEGORY) NFWK_SCRIPT_NODE_EXPLICIT(0x0000, TYPE, NAME, CATEGORY)
#define NFWK_SCRIPT_USER_NODE(TYPE, NAME, CATEGORY) NFWK_SCRIPT_NODE_EXPLICIT(0xffff, TYPE, NAME, CATEGORY)
