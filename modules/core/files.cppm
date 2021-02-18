export module nfwk.core:files;

import std.core;
import std.filesystem;

export namespace nfwk {
enum class entry_inclusion { everything, only_files, only_directories };
}

namespace nfwk {

template<typename DirectoryIterator>
std::vector<std::filesystem::path> iterate_entries_in_directory(const std::filesystem::path& path, entry_inclusion inclusion) {
	thread_local std::vector<std::filesystem::path> entries;
	entries.clear();
	std::error_code error_code{};
	for (const auto& entry : DirectoryIterator{ path, std::filesystem::directory_options::skip_permission_denied, error_code }) {
		if (inclusion != entry_inclusion::everything) {
			if (entry.is_directory() && inclusion == entry_inclusion::only_files) {
				continue;
			}
			if (!entry.is_directory() && inclusion == entry_inclusion::only_directories) {
				continue;
			}
		}
		entries.push_back(entry.path());
	}
	return entries;
}

}

export namespace nfwk {

std::vector<std::filesystem::path> entries_in_directory(std::filesystem::path path, entry_inclusion inclusion, bool recursive) {
	if (recursive) {
		return iterate_entries_in_directory<std::filesystem::recursive_directory_iterator>(path, inclusion);
	} else {
		return iterate_entries_in_directory<std::filesystem::directory_iterator>(path, inclusion);
	}
}

/*void write_file(const std::filesystem::path& path, const std::string& source) {
	std::filesystem::create_directories(path.parent_path());
	if (std::ofstream file{ path, std::ios::binary }; file.is_open()) {
		file << source;
	}
}

void write_file(const std::filesystem::path& path, const char* source, std::size_t size) {
	std::filesystem::create_directories(path.parent_path());
	if (std::ofstream file{ path, std::ios::binary }; file.is_open()) {
		file.write(source, size);
	}
}

void append_file(const std::filesystem::path& path, const std::string& source) {
	if (std::ofstream file{ path, std::ios::app }; file.is_open()) {
		file << source;
	}
}

std::string read_file(const std::filesystem::path& path) {
	if (std::ifstream file{ path, std::ios::binary }; file.is_open()) {
		std::stringstream result;
		result << file.rdbuf();
		return result.str();
	} else {
		return "";
	}
}*/

}
