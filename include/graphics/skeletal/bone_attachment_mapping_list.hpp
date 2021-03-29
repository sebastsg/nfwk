#pragma once

#include "graphics/skeletal/bone_attachment_mapping.hpp"

#include <functional>

namespace nfwk {

class model;
class bone_attachment;

class bone_attachment_mapping_list {
public:

	void save(const std::string& path);
	void load(const std::string& path);

	void for_each(const std::function<bool(bone_attachment_mapping&)>& handler);
	void remove_if(const std::function<bool(bone_attachment_mapping&)>& compare);

	bool exists(const bone_attachment_mapping& mapping) const;
	void add(const bone_attachment_mapping& mapping);
	bool update(const model& root, int animation_index, const std::u8string& attachment_model, bone_attachment& attachment) const;

	std::u8string find_root_animation(const std::u8string& root_model, const std::u8string& attached_model, const std::u8string& attached_animation) const;

private:

	std::vector<bone_attachment_mapping> mappings;

};

}
