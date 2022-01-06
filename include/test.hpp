#pragma once

#include "log.hpp"
#include "ansi_escape.hpp"

#include <vector>
#include <functional>

namespace nfwk::test {

class unit_test {
public:

	friend class test_group;

	const std::string_view name;
	int current_assert_index{ 0 };

	std::vector<std::string> messages;
	std::vector<int> indices;

	unit_test(std::string_view name);
	unit_test(const unit_test&) = delete;
	unit_test(unit_test&&) = delete;

	~unit_test() = default;

	unit_test& operator=(const unit_test&) = delete;
	unit_test& operator=(unit_test&&) = delete;

	unit_test& operator=(std::function<void()> test_function);

private:
	
	void run();

	std::function<void()> test;

};

class test_group {
public:

	friend class test_list;

	const std::string_view name;

	test_group(std::string_view name);
	test_group(test_group&&) = delete;
	test_group(const test_group&) = delete;

	~test_group() = default;

	test_group& operator=(const test_group&) = delete;
	test_group& operator=(test_group&&) = delete;

	test_group& operator=(const std::function<void()>& add_tests);

private:
	
	unit_test& add_test(std::string_view test_name);
	test_group* add_group(std::string_view group_name);

	void run();

	std::vector<std::unique_ptr<unit_test>> tests;
	std::vector<std::unique_ptr<test_group>> groups;

};

namespace internal {
void on_assert();
enum class assertion_result { true_, false_ };
}

constexpr auto passed = internal::assertion_result::true_;
constexpr auto failed = internal::assertion_result::false_;

void fail_info(std::string_view message);

void fail(std::string_view message = {}, const std::source_location& source = std::source_location::current());
internal::assertion_result fail_if(bool assertion, std::string_view message = {}, const std::source_location& source = std::source_location::current());
internal::assertion_result pass_if(bool assertion, std::string_view message = {}, const std::source_location& source = std::source_location::current());

// todo: c++20 auto

template<typename T, typename U = T>
internal::assertion_result fail_if_equal(const T& actual, const U& expected, std::string_view message = {}, const std::source_location& source = std::source_location::current()) {
	internal::on_assert();
	if (actual == expected) {
		const auto blue = ansi::sgr::call(ansi::sgr::bright_blue_text);
		const auto red = ansi::sgr::call(ansi::sgr::bright_red_text);
		fail(std::format("Actual and expected are both \"{}{}{}\"{}{}", blue, actual, red, message.empty() ? "" : " - ", message), source);
		return failed;
	} else {
		return passed;
	}
}

template<typename T, typename U = T>
internal::assertion_result fail_if_not_equal(const T& actual, const U& expected, std::string_view message = {}, const std::source_location& source = std::source_location::current()) {
	internal::on_assert();
	if (actual != expected) {
		const auto blue = ansi::sgr::call(ansi::sgr::bright_blue_text);
		const auto red = ansi::sgr::call(ansi::sgr::bright_red_text);
		fail(std::format("Expected \"{}{}{}\", but actual is \"{}{}{}\"{}{}", blue, expected, red, blue, actual, red, message.empty() ? "" : " - ", message), source);
		return failed;
	} else {
		return passed;
	}
}

bool run_tests();

unit_test& operator "" _test(const char* name, std::size_t length);
test_group& operator "" _group(const char* name, std::size_t length);

}

void test();
