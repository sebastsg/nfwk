#include "csv_view.hpp"
#include "utility_functions.hpp"

namespace nfwk {

csv_view::csv_view(std::string_view csv, char column_delimiter, char row_delimiter, std::string_view trim_characters) {
	if (csv.empty()) {
		return;
	}
	column_count = find_column_count(csv, column_delimiter, row_delimiter);
	std::size_t row_index{ 0 };
	while (row_index != std::string_view::npos) {
		std::size_t end_index{ std::string_view::npos };
		std::vector<std::string_view> current_row;
		current_row.reserve(column_count);
		bool valid_row{ true };
		for (int i{ 0 }; i < column_count - 1; i++) {
			end_index = csv.find(column_delimiter, row_index);
			if (end_index != std::string_view::npos) {
				current_row.push_back(csv.substr(row_index, end_index - row_index));
				row_index = end_index + 1;
			} else {
				valid_row = false;
				break;
			}
		}
		end_index = csv.find(row_delimiter, row_index);
		current_row.push_back(csv.substr(row_index, end_index - row_index));
		if (valid_row) {
			values.emplace_back(std::move(current_row));
		}
		if (end_index == std::string_view::npos) {
			break;
		}
		row_index = end_index + 1;
	}
	trim_fields(trim_characters);
}

int csv_view::columns() const {
	return column_count;
}

int csv_view::rows() const {
	return static_cast<int>(values.size());
}

const std::vector<std::string_view>& csv_view::row(int index) const {
	return values[index];
}

int csv_view::find_column_count(std::string_view csv, char column_delimiter, char row_delimiter) {
	const auto line_index = csv.find(row_delimiter);
	if (line_index == std::string_view::npos) {
		return 1;
	}
	int count{ 0 };
	std::size_t index{ 0 };
	do {
		count++;
		index = csv.find(column_delimiter, index + 1);
	} while (index && line_index > index);
	return count;
}

void csv_view::trim_fields(std::string_view trim_characters) {
	for (auto& row : values) {
		for (auto& field : row) {
			field = trim_string_view(field, trim_characters);
		}
	}
}

}
