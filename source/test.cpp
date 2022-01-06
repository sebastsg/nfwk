#include "test.hpp"
#include "windows_platform.hpp"
#include "utility_functions.hpp"

#include <iostream>

namespace nfwk::test {

class test_list {
public:

	test_list() = default;
	test_list(const test_list&) = delete;
	test_list(test_list&&) = delete;

	~test_list() = default;

	test_list& operator=(const test_list&) = delete;
	test_list& operator=(test_list&&) = delete;

	test_group& add_group(std::string_view name);
	unit_test& add_test(std::string_view name);

	void run();

private:

	std::vector<std::unique_ptr<test_group>> groups;

};

static const auto ansi_reset = ansi::sgr::call(ansi::sgr::reset);
static const auto ansi_white = ansi::sgr::call(ansi::sgr::white_text);
static const auto test_name_color = ansi::sgr::call(ansi::sgr::bright_yellow_text);
static const auto group_text_color = ansi::sgr::call(ansi::sgr::bright_cyan_text);
static const auto fail_text_color = ansi::sgr::call(ansi::sgr::bright_red_text);
static const auto success_text_color = ansi::sgr::call(ansi::sgr::bright_green_text);
static const auto tab_background = ansi::sgr::call(ansi::sgr::yellow_background);
static const auto tab_color = ansi::sgr::call(ansi::sgr::black_text);
static const auto value_color = ansi::sgr::call(ansi::sgr::bright_blue_text);

static std::string nesting;
static std::unique_ptr<test_list> tests;
static unit_test* current_test{ nullptr };

static std::vector<test_group*> group_stack;

static std::atomic<int> passes{ 0 };
static std::atomic<int> failures{ 0 };
static std::atomic<int> total{ 0 };

unit_test::unit_test(std::string_view name) : name{ name } {}

unit_test& unit_test::operator=(std::function<void()> test_function) {
	test = std::move(test_function);
	return *this;
}

void unit_test::run() {
	current_test = this;
	current_assert_index = 0;
	std::cout << nesting << ansi_white << "- " << test_name_color << name;
	test();
	total++;
	if (messages.empty()) {
		passes++;
		std::cout << success_text_color << " [Success]\n" << ansi_reset;
	} else {
		// todo: c++20 erase_if
		for (int i{ 0 }; i < static_cast<int>(messages.size()); i++) {
			//if (messages[i].empty()) {
			//	messages.erase(messages.begin() + i);
			//	i--;
			//}
			messages[i] += (messages[i].empty() ? "(" : " (") + std::to_string(indices[i]) + ")";
		}
		failures++;
		std::string extra_nesting{ "            " }; // "- " + " [Failed: "
		extra_nesting.insert(extra_nesting.begin(), name.size(), ' ');
		auto message = merge_strings(messages, "\n" + nesting + extra_nesting);
		replace_substring(message, "\t", std::format("{}{}\\t{}{}", tab_background, tab_color, ansi_reset, value_color));
		replace_substring(message, "\r", std::format("{}{}\\r{}{}", tab_background, tab_color, ansi_reset, value_color));
		std::cout << fail_text_color << " [Failed: " << message << "]\n" << ansi_reset;
		messages.clear();
	}
}

test_group::test_group(std::string_view name) : name{ name } {}

test_group& test_group::operator=(const std::function<void()>& add_tests) {
	group_stack.push_back(this);
	add_tests();
	group_stack.pop_back();
	return *this;
}

unit_test& test_group::add_test(std::string_view test_name) {
	if (group_stack.empty() || group_stack.back() == this) {
		return *tests.emplace_back(std::make_unique<unit_test>(test_name));
	} else {
		return group_stack.back()->add_test(test_name);
	}
}

test_group* test_group::add_group(std::string_view group_name) {
	if (!group_stack.empty() && group_stack.back() != this) {
		for (auto& group : groups) {
			if (auto new_group = group->add_group(group_name)) {
				return new_group;
			}
		}
	}
	return groups.emplace_back(std::make_unique<test_group>(group_name)).get();
}

void test_group::run() {
	std::cout << nesting << ansi_white << ">> " << group_text_color << name << ansi_reset << '\n';
	nesting += "    ";
	for (auto& test : tests) {
		test->run();
	}
	for (auto& group : groups) {
		group->run();
	}
	nesting.pop_back();
	nesting.pop_back();
	nesting.pop_back();
	nesting.pop_back();
}

test_group& test_list::add_group(std::string_view name) {
	if (group_stack.empty()) {
		return *groups.emplace_back(std::make_unique<test_group>(name));
	} else {
		return *group_stack.back()->add_group(name);
	}
}

unit_test& test_list::add_test(std::string_view name) {
	if (group_stack.empty()) {
		return groups.emplace_back(std::make_unique<test_group>(""))->add_test(name);
	} else {
		return group_stack.back()->add_test(name);
	}
}

void test_list::run() {
	std::cout << ansi_white << "[Tests]\n" << ansi_reset;
	for (auto& group : groups) {
		group->run();
	}
	std::cout << success_text_color << std::format("{}/{} tests passed.", passes.load(), total.load()) << ansi_reset << std::endl;
	if (failures != 0) {
		std::cout << fail_text_color << std::format("{} tests failed.", failures.load()) << ansi_reset << std::endl;
	}
}

namespace internal {

void on_assert() {
	current_test->current_assert_index++;
}

}

void fail_info(std::string_view message) {
	if (current_test->messages.empty()) {
		current_test->messages.emplace_back(message);
	} else {
		current_test->messages.back() += message;
	}
}

void fail(std::string_view message, const std::source_location& source) {
	current_test->messages.emplace_back(message);
	current_test->indices.push_back(current_test->current_assert_index);
}

internal::assertion_result fail_if(bool assertion, std::string_view message, const std::source_location& source) {
	internal::on_assert();
	if (assertion) {
		fail(message, source);
		return failed;
	} else {
		return passed;
	}
}

internal::assertion_result pass_if(bool assertion, std::string_view message, const std::source_location& source) {
	internal::on_assert();
	if (assertion) {
		return passed;
	} else {
		fail(message, source);
		return failed;
	}
}

bool run_tests() {
	platform::windows::initialize_console();
	total = 0;
	failures = 0;
	passes = 0;
	tests = std::make_unique<test_list>();
	::test();
	tests->run();
	tests = nullptr;
	return failures == 0;
}

unit_test& operator "" _test(const char* name, std::size_t length) {
	return tests->add_test(name);
}

test_group& operator "" _group(const char* name, std::size_t length) {
	return tests->add_group(name);
}

}
