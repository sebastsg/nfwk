#include "log.hpp"
#include "loop.hpp"
#include "graphics/ui.hpp"
#include "utility_functions.hpp"

#include <chrono>
#include <memory>
#include <typeinfo>

std::ostream& operator<<(std::ostream& out, nfwk::log::entry_type message) {
	switch (message) {
	case nfwk::log::entry_type::message: return out << "message";
	case nfwk::log::entry_type::warning: return out << "warning";
	case nfwk::log::entry_type::error: return out << "error";
	case nfwk::log::entry_type::info: return out << "info";
	}
}

std::ostream& operator<<(std::ostream& out, const std::vector<std::u8string>& strings) {
	for (std::size_t i{ 0 }; i < strings.size(); i++) {
		out << std::string{ strings[i].begin(), strings[i].end() };
		if (strings.size() - 1 > i) {
			out << ", ";
		}
	}
	return out;
}

namespace nfwk::log {

static std::vector<std::shared_ptr<debug_log>> all_logs;
static std::vector<std::shared_ptr<log_writer>> writers;

log_writer::~log_writer() {}

debug_log::debug_log(std::u8string_view name, const std::vector<std::shared_ptr<debug_log>>& logs) : name{ name } {
	for (const auto& log : logs) {
		entries = merge_vectors(entries, log->get_entries());
	}
	std::sort(entries.begin(), entries.end(), [](const log_entry& a, const log_entry& b) {
		return a.timestamp < b.timestamp;
	});
}

std::vector<std::shared_ptr<log_writer>> debug_log::get_writers() const {
	std::vector<std::shared_ptr<log_writer>> result;
	for (auto& writer : writers) {
		if (writer->get_log().get() == this) {
			result.push_back(writer);
		}
	}
	return result;
}

void add_writer(const std::shared_ptr<log_writer>& new_writer) {
	const auto& new_writer_type = typeid(*new_writer);
	auto it = std::find_if(writers.begin(), writers.end(), [&](const std::shared_ptr<log_writer>& existing_writer) {
		return existing_writer->get_log() == new_writer->get_log() && typeid(*existing_writer) == new_writer_type;
	});
	if (it == writers.end()) {
		writers.push_back(new_writer);
	}
}

void add_log(std::u8string_view name) {
	for (const auto& log : all_logs) {
		if (log->name == name) {
			ASSERT(false); 
			return;
		}
	}
	all_logs.emplace_back(std::make_shared<debug_log>(name));
}

void add_entry(const log_entry_identifier& identifier, entry_type type, std::u8string_view message) {
	for (auto& log : all_logs) {
		if (log->name == identifier.id) {
			log->add(type, message, identifier.source.file_name(), identifier.source.function_name(), identifier.source.line());
			return;
		}
	}
	// log wasn't added, so we add the log and try again.
	add_log(identifier.id);
	add_entry(identifier, type, message);
}

std::vector<std::shared_ptr<debug_log>>& get_logs() {
	return all_logs;
}

std::shared_ptr<debug_log> find_log(const std::u8string& name) {
	for (auto& log : all_logs) {
		if (log->name == name) {
			return log;
		}
	}
	return nullptr;
}

std::u8string current_local_time_string() {
	const std::time_t now{ std::time(nullptr) };
	tm local_time;
	localtime_s(&local_time, &now);
	char8_t buffer[64];
	std::strftime(reinterpret_cast<char*>(buffer), 64, "%X", &local_time);
	return buffer;
}

std::u8string current_local_date_string() {
	const std::time_t now{ std::time(nullptr) };
	tm local_time;
	localtime_s(&local_time, &now);
	char8_t buffer[64];
	std::strftime(reinterpret_cast<char*>(buffer), 64, "%Y.%m.%d", &local_time);
	return buffer;
}

std::u8string current_time_string_for_log() {
	const auto ms = current_time<std::chrono::milliseconds>() % 1000;
	return current_local_time_string() + u8"." + to_string(ms);
}

}
