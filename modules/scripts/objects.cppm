module;

#include "assert.hpp"

export module nfwk.scripts:objects;

import std.core;
import std.memory;
import std.filesystem;
import nfwk.core;
import nfwk.assets;
import :object_instance;
import :object_class;
import :object_renderer;

namespace nfwk::objects {

std::vector<std::unique_ptr<object_class>> classes;
std::vector<object_instance> instances;

}

export namespace nfwk::objects {

void initialize() {
	register_object_component<object_sprite_component>();
}

std::optional<int> find_unused_class_index() {
	for (int index{ 0 }; index < static_cast<int>(classes.size()); index++) {
		if (!classes[index]) {
			return index;
		}
	}
	return std::nullopt;
}

std::optional<int> find_unused_instance_index() {
	for (int index{ 0 }; index < static_cast<int>(instances.size()); index++) {
		if (!instances[index].is_alive()) {
			return index;
		}
	}
	return std::nullopt;
}

void register_loaded_class(std::unique_ptr<object_class> class_) {
	const auto index = class_->get_index();
	if (index < 0) {
		warning("main", "Negative index {} in class {}", index, class_->get_id());
		return;
	}
	while (index >= static_cast<int>(classes.size())) {
		classes.emplace_back();
	}
	if (classes[index]) {
		warning("main", "Duplicate index {}!\nOld: {}\nNew: {}", index, classes[index]->get_id(), class_->get_id());
		return;
	}
	classes[index] = std::move(class_);
}

void load_class(const std::filesystem::path& path) {
	//if (path.extension().string() != object_class::file_extension) {
	//	warning("main", "Not an object file: {}", path);
	//	return;
	//}
	if (io_stream stream{ path }; !stream.empty()) {
		register_loaded_class(std::make_unique<object_class>(stream));
	} else {
		warning("main", "Ignoring empty file: {}", path);
	}
}

void spawn(int class_index) {
	if (const auto unused_index = find_unused_instance_index()) {
		classes[class_index]->remake_instance(instances[unused_index.value()]);
	} else {
		instances.emplace_back(classes[class_index]->make_instance());
	}
}

const std::vector<object_instance>& get_instances() {
	return instances;
}

const std::vector<std::unique_ptr<object_class>>& get_classes() {
	return classes;
}

object_class* get_class(int index) {
	return index >= 0 && index < static_cast<int>(classes.size()) ? classes[index].get() : nullptr;
}

bool class_exists(int index) {
	return index >= 0 && index < static_cast<int>(classes.size()) && classes[index];
}

std::optional<int> find_class(std::string_view class_id) {
	for (const auto& class_ : classes) {
		if (class_->get_id() == class_id) {
			return class_->get_index();
		}
	}
	return std::nullopt;
}

std::optional<int> register_class(std::string_view class_id) {
	if (find_class(class_id)) {
		warning("main", "{} is already registered.", class_id);
		return std::nullopt;
	}
	if (const auto unused_index = find_unused_class_index()) {
		const auto index = unused_index.value();
		classes[index] = std::make_unique<object_class>(index, class_id);
		return index;
	} else {
		const auto index = static_cast<int>(classes.size());
		classes.emplace_back(std::make_unique<object_class>(index, class_id));
		return index;
	}
}

bool set_class_id(int class_index, std::string_view new_id) {
	ASSERT(class_exists(class_index));
	if (find_class(new_id)) {
		return false;
	}
	classes[class_index]->change_id(new_id);
	return true;
}

std::optional<std::string_view> get_class_id(int class_index) {
	return classes[class_index]->get_id();
}

[[nodiscard]] std::vector<object_class*> search_class(const std::string& search_term, int limit) {
	std::vector<object_class*> results;
	const auto& search = string_to_lowercase(search_term);
	for (auto& class_ : classes) {
		const auto& event_id = string_to_lowercase(std::string{ class_->get_id() });
		if (event_id.find(search) != std::string::npos) {
			results.push_back(class_.get());
		}
		if (static_cast<int>(results.size()) >= limit) {
			break;
		}
	}
	return results;
}

void save_class(int class_index) {
	io_stream stream;
	classes[class_index]->write(stream);
	write_file(classes[class_index]->get_path(), stream);
}

void save_classes() {
	for (int index{ 0 }; index < static_cast<int>(classes.size()); index++) {
		save_class(index);
	}
}

void load_classes() {
	for (const auto& path : entries_in_directory(asset_path("objects"), entry_inclusion::only_files, false)) {
		load_class(path);
	}
}

}
