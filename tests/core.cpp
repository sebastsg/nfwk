#include "csv_view.hpp"
#include "test.hpp"
#include "utility_functions.hpp"
#include "regex.hpp"
#include "url.hpp"

namespace nfwk::test {

void utility_function_tests() {
	
	"split_string()"_test = [] {
		const std::string input{ "This is a string." };
		const auto& words = split_string(input, ' ');
		fail_if(words.size() != 4);
		fail_if(words[0] != "This");
		fail_if(words[1] != "is");
		fail_if(words[2] != "a");
		fail_if(words[3] != "string.");
	};

	"string_to_lowercase()"_test = [] {
		const std::string input{ "This is a string with UPPERCASE LETTERS" };
		const auto& output = string_to_lowercase(input);
		fail_if_not_equal(output, "this is a string with uppercase letters");
	};

	"Editing substrings"_group = [] {

		"replace_substring() defaults"_test = [] {
			std::string input{ "The brown cat is eating a fish, and the cat is enjoying it." };
			replace_substring(input, "cat is", "birds are");
			fail_if_not_equal(input, "The brown birds are eating a fish, and the birds are enjoying it.");
		};

		"replace_substring() various offsets"_test = [] {
			std::string input{ "The brown cat is eating a fish." };
			replace_substring(input, "cat is", "birds are", 12);
			fail_if_not_equal(input, "The brown cat is eating a fish.");
			replace_substring(input, "cat is", "birds are", 6);
			fail_if_not_equal(input, "The brown birds are eating a fish.");
		};

		"replace_substring() max replacements"_test = [] {
			std::string input{ "The brown cat and brown birds are eating a brown fish." };
			replace_substring(input, "brown", "purple", 0, 2);
			fail_if_not_equal(input, "The purple cat and purple birds are eating a brown fish.");
		};

		"replace_substring() max replacements with an offset"_test = [] {
			std::string input{ "The brown cat and brown birds are eating a brown fish." };
			replace_substring(input, "brown", "purple", 5, 2);
			fail_if_not_equal(input, "The brown cat and purple birds are eating a purple fish.");
		};
		
	};

	"Converting from string to T"_group = [] {

		"from_string<int>() number followed by characters"_test = [] {
			constexpr std::string_view input{ "100?? That's great!" };
			const auto output = from_string<int>(input);
			fail_if_not_equal(output.value_or(0), 100);
		};

		"from_string<int>() characters followed by number"_test = [] {
			constexpr std::string_view input{ "That's great, 100!" };
			const auto output = from_string<int>(input);
			fail_if(output.has_value());
		};

		"from_string<int>() only characters"_test = [] {
			constexpr std::string_view input{ "Wow. That's great!" };
			const auto output = from_string<int>(input);
			fail_if(output.has_value());
		};

	};

	"String trimming"_group = [] {

		"trim_string_left()"_test = [] {
			std::string string{ "... That's interesting! . ." };
			trim_string_left(string, " .");
			fail_if_not_equal(string, "That's interesting! . .");
		};

		"trim_string_right()"_test = [] {
			std::string string{ "... That's interesting! . . " };
			trim_string_right(string, " .");
			fail_if_not_equal(string, "... That's interesting!");
		};

		"trim_string()"_test = [] {
			std::string string{ "That's interesting!" };
			trim_string(string, " .");
			fail_if_not_equal(string, "That's interesting!");
		};

		"trim_string_view_left()"_test = [] {
			std::string_view string{ "... That's interesting! . ." };
			string = trim_string_view_left(string, " .");
			fail_if_not_equal(string, "That's interesting! . .");
		};

		"trim_string_view_right()"_test = [] {
			std::string_view string{ "... That's interesting! . . " };
			string = trim_string_view_right(string, " .");
			fail_if_not_equal(string, "... That's interesting!");
		};

		"trim_string_view()"_test = [] {
			std::string_view string{ "That's interesting!" };
			string = trim_string_view(string, " .");
			fail_if_not_equal(string, "That's interesting!");
		};

	};

	"Erase multispace in strings"_group = [] {

		"erase_multi_space() only spaces"_test = [] {
			std::string string{ "Such a  cool     idea! " };
			erase_multi_occurrence(string, { '\t', ' ', '-' }, ' ');
			fail_if_not_equal(string, "Such a cool idea! ");
		};

		"erase_multi_space() only tabs"_test = [] {
			std::string string{ "Such\ta\t\tcool\t\t\t\t\tidea!\t" };
			erase_multi_occurrence(string, { '\t', ' ', 'S' }, ' ');
			fail_if_not_equal(string, "Such\ta cool idea!\t");
		};

		"erase_multi_space() tabs and spaces #1"_test = [] {
			std::string string{ "Such a \tcool \t \t idea! " };
			erase_multi_occurrence(string, { '\t', ' ' }, ' ');
			fail_if_not_equal(string, "Such a cool idea! ");
		};

		"erase_multi_space() tabs and spaces #2"_test = [] {
			std::string string{ "Such a\t\tcool   \t idea! \t" };
			erase_multi_occurrence(string, { '\t', ' ' }, ' ');
			fail_if_not_equal(string, "Such a cool idea! ");
		};

		"erase_multi_space() dashes"_test = [] {
			std::string string{ "A -- very ----- great - idea!" };
			erase_multi_occurrence(string, { '-' }, '-');
			fail_if_not_equal(string, "A - very - great - idea!");
		};

	};
	
}

void regex_tests() {
	"find_in()"_test = [] {
		compiled_regex regex{ "\\(?\\d+\\s*[-.]?\\)?" };
		fail_if_not_equal(regex.find_in("01. Some text").value_or(""), "01.");
		fail_if_not_equal(regex.find_in("01 - Some text").value_or(""), "01 -");
		fail_if_not_equal(regex.find_in("1. Some text").value_or(""), "1.");
		fail_if_not_equal(regex.find_in("1 - Some text").value_or(""), "1 -");
	};
	"find_all_in()"_test = [] {
		compiled_regex regex{ "\\(?\\d+\\s*[-.]?\\)?" };
		fail_if_not_equal(regex.find_all_in("01.Some 04  -text"), { "01.", "04  -" });
		fail_if_not_equal(regex.find_all_in("Some text9-10-"), { "9-", "10-" });
	};
}

void io_stream_tests() {
	"Write data to stream, copy it to a new stream, and read it back"_test = [] {
		io_stream in;
		in.write_int8(10);
		in.write_int16(-250);
		in.write_int32(900);
		in.write_int64(1000);
		in.write_bool(true);
		in.write_string("Example text");
		in.write_array<float>({ 0.0f, 1.0f, 5.0f, -10.0f });
		in.write_optional<std::int32_t>(std::optional<std::int32_t>{});
		in.write_optional<std::int32_t>(std::optional<std::int32_t>{ 50 });

		io_stream out{ in.data(), in.size_left_to_read(), io_stream::construct_by::shallow_copy };
		fail_if_not_equal(out.read_int8(), 10);
		fail_if_not_equal(out.read_int16(), -250);
		fail_if_not_equal(out.read_int32(), 900);
		fail_if_not_equal(out.read_int64(), 1000);
		fail_if_not_equal(out.read_bool(), true);
		fail_if_not_equal(out.read_string(), "Example text");
		fail_if_not_equal(out.read_array<float>(), { 0.0f, 1.0f, 5.0f, -10.0f });
		fail_if(out.read_optional<std::int32_t>().has_value());
		fail_if_not_equal(out.read_optional<std::int32_t>().value_or(0), 50);
	};

	"Validate read and write indices"_test = [] {
		io_stream stream;
		fail_if_not_equal(stream.write_index(), 0);
		fail_if_not_equal(stream.read_index(), 0);

		// write some values
		stream.write_int32(500);
		stream.write_int64(1000);
		fail_if_not_equal(stream.write_index(), sizeof(std::int32_t) + sizeof(std::int64_t));
		fail_if_not_equal(stream.read_index(), 0);
		fail_if_not_equal(stream.size_left_to_read(), sizeof(std::int32_t) + sizeof(std::int64_t));

		// read back values
		fail_if_not_equal(stream.read_int32(), 500);
		fail_if_not_equal(stream.size_left_to_read(), sizeof(std::int64_t));
		fail_if_not_equal(stream.read_index(), sizeof(std::int32_t));
		fail_if_not_equal(stream.write_index(), sizeof(std::int32_t) + sizeof(std::int64_t));
		
		fail_if_not_equal(stream.read_int64(), 1000);
		fail_if_not_equal(stream.size_left_to_read(), 0);
		fail_if_not_equal(stream.read_index(), sizeof(std::int32_t) + sizeof(std::int64_t));
		fail_if_not_equal(stream.write_index(), sizeof(std::int32_t) + sizeof(std::int64_t));

		// write another value
		stream.write_int16(250);
		fail_if_not_equal(stream.size_left_to_read(), sizeof(std::int16_t));
		fail_if_not_equal(stream.read_index(), sizeof(std::int32_t) + sizeof(std::int64_t));
		fail_if_not_equal(stream.write_index(), sizeof(std::int32_t) + sizeof(std::int64_t) + sizeof(std::int16_t));

		// read it back
		fail_if_not_equal(stream.read_int16(), 250);
		fail_if_not_equal(stream.size_left_to_read(), 0);
		fail_if_not_equal(stream.read_index(), sizeof(std::int32_t) + sizeof(std::int64_t) + sizeof(std::int16_t));
		fail_if_not_equal(stream.write_index(), sizeof(std::int32_t) + sizeof(std::int64_t) + sizeof(std::int16_t));
	};

	"Replace substring, expand string"_test = [] {
		// prepare
		constexpr std::string_view input{ "The brown cat is eating a fish." };
		io_stream stream;
		stream.write_raw(input);
		fail_if_not_equal(stream.write_index(), input.size());

		// replace
		const auto offset = stream.replace("cat is", "birds are");

		// verify
		constexpr std::string_view expected_output{ "The brown birds are eating a fish." };
		pass_if(offset.has_value());
		fail_if_not_equal(offset.value_or(0), std::string_view{ "The brown birds are" }.size());
		const auto output = stream.get_string_view();
		fail_if_not_equal(output, expected_output);
		fail_if_not_equal(stream.read_index(), 0);
		fail_if_not_equal(stream.write_index(), expected_output.size());
	};

	"Replace substring, shrink string"_test = [] {
		// prepare
		constexpr std::string_view input{ "The brown cat is eating a fish." };
		io_stream stream;
		stream.write_raw(input);
		fail_if_not_equal(stream.write_index(), input.size());

		// replace
		const auto offset = stream.replace("brown cat is eating", "birds eat");

		// verify
		constexpr std::string_view expected_output{ "The birds eat a fish." };
		pass_if(offset.has_value());
		fail_if_not_equal(offset.value_or(0), std::string_view{ "The birds eat" }.size());
		const auto output = stream.get_string_view();
		fail_if_not_equal(output, expected_output);
		fail_if_not_equal(stream.read_index(), 0);
		fail_if_not_equal(stream.write_index(), expected_output.size());
	};

	"Replace substring, expand string, multiple times"_test = [] {
		// prepare
		constexpr std::string_view input{ "The brown cat is eating a fish cooked by a cat and caught by a cat who was very nice." };
		io_stream stream;
		stream.write_raw(input);
		fail_if_not_equal(stream.write_index(), input.size());

		// replace
		const auto offset = stream.replace("cat", "really long name for an animal");

		// verify
		constexpr std::string_view expected_output{ "The brown really long name for an animal is eating a fish cooked by a really long name for an animal and caught by a really long name for an animal who was very nice." };
		pass_if(offset.has_value());
		fail_if_not_equal(offset.value_or(0), std::string_view{ "The brown really long name for an animal is eating a fish cooked by a really long name for an animal and caught by a really long name for an animal" }.size());
		const auto output = stream.get_string_view();
		fail_if_not_equal(output, expected_output);
		fail_if_not_equal(stream.read_index(), 0);
		fail_if_not_equal(stream.write_index(), expected_output.size());
	};

	// todo: duplicate above but add an offset and a max replacements
	
}

void csv_tests() {

	"1x3, ends without row separator"_test = [] {
		constexpr std::string_view csv{
			"Column 1 Row 1\n"
			"Column 1 Row 2\n"
			"Column 1 Row 3"
		};
		csv_view view{ csv, ';', '\n', "" };
		fail_if_not_equal(view.columns(), 1);
		fail_if_not_equal(view.rows(), 3);
		fail_if_not_equal(view.row(0)[0], "Column 1 Row 1");
		fail_if_not_equal(view.row(1)[0], "Column 1 Row 2");
		fail_if_not_equal(view.row(2)[0], "Column 1 Row 3");
	};

	"2x2, ends without row separator"_test = [] {
		constexpr std::string_view csv{
			"Column 1 Row 1;Column 2 Row 1\n"
			"Column 1 Row 2;Column 2 Row 2"
		};
		csv_view view{ csv, ';', '\n', "" };
		fail_if_not_equal(view.columns(), 2);
		fail_if_not_equal(view.rows(), 2);
		fail_if_not_equal(view.row(0)[0], "Column 1 Row 1");
		fail_if_not_equal(view.row(0)[1], "Column 2 Row 1");
		fail_if_not_equal(view.row(1)[0], "Column 1 Row 2");
		fail_if_not_equal(view.row(1)[1], "Column 2 Row 2");
	};

	"2x2, ends with multiple row separators"_test = [] {
		constexpr std::string_view csv{
			"Column 1 Row 1;Column 2 Row 1\n"
			"Column 1 Row 2;Column 2 Row 2\n\n\n"
		};
		csv_view view{ csv, ';', '\n', "" };
		fail_if_not_equal(view.columns(), 2);
		fail_if_not_equal(view.rows(), 2);
		fail_if_not_equal(view.row(0)[0], "Column 1 Row 1");
		fail_if_not_equal(view.row(0)[1], "Column 2 Row 1");
		fail_if_not_equal(view.row(1)[0], "Column 1 Row 2");
		fail_if_not_equal(view.row(1)[1], "Column 2 Row 2");
	};

	"2x2, ends with multiple row separators, trim \\r"_test = [] {
		constexpr std::string_view csv{
			"Column 1 Row 1;Column 2 Row 1\r\n"
			"Column 1 Row 2;Column 2 Row 2\r\n\r\n\r\n"
		};
		csv_view view{ csv, ';', '\n', "\r" };
		fail_if_not_equal(view.columns(), 2);
		fail_if_not_equal(view.rows(), 2);
		fail_if_not_equal(view.row(0)[0], "Column 1 Row 1");
		fail_if_not_equal(view.row(0)[1], "Column 2 Row 1");
		fail_if_not_equal(view.row(1)[0], "Column 1 Row 2");
		fail_if_not_equal(view.row(1)[1], "Column 2 Row 2");
	};

	"3x3, ends with multiple row separators, trim spaces, tabs, and \\r"_test = [] {
		constexpr std::string_view csv{
			"   Column 1 Row 1;   \tColumn 2 Row 1 ; Column 3 Row 1  \r\n"
			"   Column 1 Row 2;   \tColumn 2 Row 2  ; Column 3 Row 2\r\n"
			"  \t  Column 1 Row 3;\t   Column 2 Row 3;Column 3 Row 3\t\r\n\r\n\r\n"
		};
		csv_view view{ csv, ';', '\n', "\t\r " };
		fail_if_not_equal(view.columns(), 3);
		fail_if_not_equal(view.rows(), 3);
		fail_if_not_equal(view.row(0)[0], "Column 1 Row 1");
		fail_if_not_equal(view.row(0)[1], "Column 2 Row 1");
		fail_if_not_equal(view.row(0)[2], "Column 3 Row 1");
		fail_if_not_equal(view.row(1)[0], "Column 1 Row 2");
		fail_if_not_equal(view.row(1)[1], "Column 2 Row 2");
		fail_if_not_equal(view.row(1)[2], "Column 3 Row 2");
		fail_if_not_equal(view.row(2)[0], "Column 1 Row 3");
		fail_if_not_equal(view.row(2)[1], "Column 2 Row 3");
		fail_if_not_equal(view.row(2)[2], "Column 3 Row 3");
	};
	
}

void url_tests() {

	"decode_url()"_test = [] {
		fail_if_not_equal(decode_url("%20some%20%20%20spaces%20here%20"), " some   spaces here ");
		fail_if_not_equal(decode_url("wh%C3%BCt"), nfwk::from_u8(u8"whüt"));
		fail_if_not_equal(decode_url("%E3%81%AD%E3%81%93"), nfwk::from_u8(u8"ねこ"));
	};
	
}

void file_type_tests() {

	"find_file_type_from_data() mp3 expected"_test = [] {
		io_stream stream;
		stream.write_int32(0b00000000000000000000011111111111);
		const auto type = detect_file_type_from_data(stream.get_string_view());
		if (pass_if(type.has_value()) == passed) {
			fail_if_not_equal(type.value(), file_type::mp3);
		}
	};

	"find_file_type_from_data() mp3 not expected"_test = [] {
		constexpr std::uint32_t headers[]{
			0b00000000000000000000001111111111,
			0b11111111111111111111110000000000,
			0b11111111111111111111010100000000,
			0b10101010111111111111010100000001,
			0b00000000000000000000000000000000,
		};
		io_stream stream;
		for (const auto header : headers) {
			stream.write_int32(header);
			const auto type = detect_file_type_from_data(stream.get_string_view());
			if (fail_if(type.has_value()) == failed) {
				fail_info(std::format("File Type: {}. (Extension: {})", enum_string(type), get_file_extension_from_type(type.value(), true)));
			}
			stream.reset();
		}
	};

	"detect_file_type_from_extension() fail"_test = [] {
		constexpr std::string_view extensions[]{ ".", ".." };
		for (const auto extension : extensions) {
			const auto type = detect_file_type_from_extension(extension);
			fail_if(type.has_value());
		}
	};

	"detect_file_type_from_extension() images - pass"_test = [] {
		constexpr std::string_view extensions[]{
			"..jpg", "jpg", ".JPG", "png", ".Png", "jpeg", ".jpeG",
			"gif", ".gIF", "webp", "bmp", ".webP", ".BMP", ".JPEG"
		};
		for (const auto extension : extensions) {
			const auto type = detect_file_type_from_extension(extension);
			if (pass_if(type.has_value()) == passed) {
				fail_if_not_equal(get_file_type_category(type.value()), file_type_category::image);
			}
		}
	};

	"detect_file_type_from_extension() audio - pass"_test = [] {
		constexpr std::string_view extensions[]{
			"..flac", "flac", ".FLAC", "mp3", ".Mp3", "m4a",
			".M4A", "wav", ".WAV", "ogg", ".oGg", ".MP3"
		};
		for (const auto extension : extensions) {
			const auto type = detect_file_type_from_extension(extension);
			if (pass_if(type.has_value()) == passed) {
				fail_if_not_equal(get_file_type_category(type.value()), file_type_category::audio);
			}
		}
	};
	
}

void test_core() {
	"Utility Functions"_group = utility_function_tests;
	"Regex"_group = regex_tests;
	"I/O Stream"_group = io_stream_tests;
	"CSV"_group = csv_tests;
	"URL"_group = url_tests;
	"File Type"_group = file_type_tests;
}

}
