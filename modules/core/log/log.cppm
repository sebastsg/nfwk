export module nfwk.core:log;

import std.core;
import std.memory;
import std.threading;
import std.filesystem;
export import :log.core;
export import :log.html_writer;
export import :log.stdout_writer;

namespace nfwk::log {

std::unordered_map<std::string, debug_log> logs;
std::vector<std::unique_ptr<log_writer>> writers;

void add_writer(const std::string& name, std::unique_ptr<log_writer> writer) {
	writers.emplace_back(std::move(writer));
}

}

export namespace nfwk::log {

void start_logging() {
	//if (auto buffer = read_file(asset_path("debug/template.html")); !buffer.empty()) {
	//	template_html = buffer;
	//}
}

void stop_logging() {
	writers.clear();
	logs.clear();
}

void add_entry(std::string_view name_, entry_type type, std::string_view file, std::string_view function, int line, std::string_view message) {
	std::string name{ name_ };
	const auto& [log, _] = logs.try_emplace(name, name);
	log->second.add(type, message, file, function, line);
}

std::optional<std::reference_wrapper<debug_log>> find_log(const std::string& name) {
	if (auto log = logs.find(name); log != logs.end()) [[likely]] {
		return log->second;
	} else {
		return std::nullopt;
	}
}

template<typename Writer>
void add_writer(const std::string& name) {
	if (auto log = find_log(name)) {
		add_writer(name, std::make_unique<Writer>(log->get()));
	}
}

}

// temporary until supported
export namespace std {
class source_location {
public:
	std::string file_name() const {
		return "File";
	}
	std::string function_name() const {
		return "Function";
	}
	int line() const {
		return 1;
	}
	static source_location current() {
		return {};
	}
};
}


export namespace nfwk {

void message(std::string_view id, std::string_view message, const std::source_location& source = std::source_location::current()) {
	log::add_entry(id, log::entry_type::message, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0>
void message(std::string_view id, std::string_view format, Arg0 arg0, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0);
	log::add_entry(id, log::entry_type::message, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0, typename Arg1>
void message(std::string_view id, std::string_view format, Arg0 arg0, Arg1 arg1, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0, arg1);
	log::add_entry(id, log::entry_type::message, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0, typename Arg1, typename Arg2>
void message(std::string_view id, std::string_view format, Arg0 arg0, Arg1 arg1, Arg2 arg2, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0, arg1, arg2);
	log::add_entry(id, log::entry_type::message, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0, typename Arg1, typename Arg2, typename Arg3>
void message(std::string_view id, std::string_view format, Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0, arg1, arg2, arg3);
	log::add_entry(id, log::entry_type::message, source.file_name(), source.function_name(), source.line(), message);
}

void info(std::string_view id, std::string_view message, const std::source_location& source = std::source_location::current()) {
	log::add_entry(id, log::entry_type::info, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0>
void info(std::string_view id, std::string_view format, Arg0 arg0, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0);
	log::add_entry(id, log::entry_type::info, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0, typename Arg1>
void info(std::string_view id, std::string_view format, Arg0 arg0, Arg1 arg1, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0, arg1);
	log::add_entry(id, log::entry_type::info, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0, typename Arg1, typename Arg2>
void info(std::string_view id, std::string_view format, Arg0 arg0, Arg1 arg1, Arg2 arg2, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0, arg1, arg2);
	log::add_entry(id, log::entry_type::info, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0, typename Arg1, typename Arg2, typename Arg3>
void info(std::string_view id, std::string_view format, Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0, arg1, arg2, arg3);
	log::add_entry(id, log::entry_type::info, source.file_name(), source.function_name(), source.line(), message);
}

void warning(std::string_view id, std::string_view format, const std::source_location& source = std::source_location::current()) {
	log::add_entry(id, log::entry_type::warning, source.file_name(), source.function_name(), source.line(), format);
}

template<typename Arg0>
void warning(std::string_view id, std::string_view format, Arg0 arg0, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0);
	log::add_entry(id, log::entry_type::warning, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0, typename Arg1>
void warning(std::string_view id, std::string_view format, Arg0 arg0, Arg1 arg1, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0, arg1);
	log::add_entry(id, log::entry_type::warning, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0, typename Arg1, typename Arg2>
void warning(std::string_view id, std::string_view format, Arg0 arg0, Arg1 arg1, Arg2 arg2, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0, arg1, arg2);
	log::add_entry(id, log::entry_type::warning, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0, typename Arg1, typename Arg2, typename Arg3>
void warning(std::string_view id, std::string_view format, Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0, arg1, arg2, arg3);
	log::add_entry(id, log::entry_type::warning, source.file_name(), source.function_name(), source.line(), message);
}

void error(std::string_view id, std::string_view format, const std::source_location& source = std::source_location::current()) {
	log::add_entry(id, log::entry_type::error, source.file_name(), source.function_name(), source.line(), format);
}

template<typename Arg0>
void error(std::string_view id, std::string_view format, Arg0 arg0, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0);
	log::add_entry(id, log::entry_type::error, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0, typename Arg1>
void error(std::string_view id, std::string_view format, Arg0 arg0, Arg1 arg1, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0, arg1);
	log::add_entry(id, log::entry_type::error, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0, typename Arg1, typename Arg2>
void error(std::string_view id, std::string_view format, Arg0 arg0, Arg1 arg1, Arg2 arg2, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0, arg1, arg2);
	log::add_entry(id, log::entry_type::error, source.file_name(), source.function_name(), source.line(), message);
}

template<typename Arg0, typename Arg1, typename Arg2, typename Arg3>
void error(std::string_view id, std::string_view format, Arg0 arg0, Arg1 arg1, Arg2 arg2, Arg3 arg3, const std::source_location& source = std::source_location::current()) {
	auto message = format; //auto message = std::format(format, arg0, arg1, arg2, arg3);
	log::add_entry(id, log::entry_type::error, source.file_name(), source.function_name(), source.line(), message);
}

}
