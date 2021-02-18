export module nfwk.scripts:script_utility_functions;

import std.core;

export namespace nfwk {

[[nodiscard]] std::string normalized_identifier(std::string_view id) {
	std::string normalized_id{ id };
	for (auto& character : normalized_id) {
		if (!std::isalnum(character)) {
			character = '-';
		}
	}
	return normalized_id;
}

[[nodiscard]] bool is_identifier_normalized(std::string_view id) {
	return normalized_identifier(id) == id;
}

}
