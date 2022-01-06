#pragma once

#include <vector>
#include <string_view>

namespace nfwk {

class csv_view {
public:

	csv_view(std::string_view csv, char column_delimiter, char row_delimiter, std::string_view trim_characters);
	csv_view(const csv_view&) = delete;
	csv_view(csv_view&&) = delete;

	~csv_view() = default;

	csv_view& operator=(const csv_view&) = delete;
	csv_view& operator=(csv_view&&) = delete;

	int columns() const;
	int rows() const;
	
	const std::vector<std::string_view>& row(int index) const;

private:

	static int find_column_count(std::string_view csv, char column_delimiter, char row_delimiter);

	void trim_fields(std::string_view trim_characters);

	int column_count{ 0 };
	std::vector<std::vector<std::string_view>> values;
	
};

}
