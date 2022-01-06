#include "graphics/skeletal/bone_attachment_mapping_list.hpp"
#include "graphics/skeletal/bone_attachment.hpp"
#include "graphics/model.hpp"
#include "io.hpp"

namespace nfwk {

void bone_attachment_mapping_list::save(const std::string& path) {
	io_stream stream;
	stream.write_size(mappings.size());
	for (auto& attachment : mappings) {
		stream.write_string(attachment.root_model);
		stream.write_string(attachment.root_animation);
		stream.write_string(attachment.attached_model);
		stream.write_string(attachment.attached_animation);
		stream.write_int32(attachment.attached_to_channel);
		stream.write_struct(attachment.position);
		stream.write_float32(attachment.rotation.x);
		stream.write_float32(attachment.rotation.y);
		stream.write_float32(attachment.rotation.z);
		stream.write_float32(attachment.rotation.w);
	}
	write_file(path, stream);
}

void bone_attachment_mapping_list::load(const std::string& path) {
	io_stream stream;
	read_file(path, stream);
	if (stream.write_index() == 0) {
		return;
	}
	const auto count = stream.read_size();
	for (std::size_t i{ 0 }; i < count; i++) {
		bone_attachment_mapping mapping;
		mapping.root_model = stream.read_string();
		mapping.root_animation = stream.read_string();
		mapping.attached_model = stream.read_string();
		mapping.attached_animation = stream.read_string();
		mapping.attached_to_channel = stream.read_int32();
		mapping.position = stream.read_struct<vector3f>();
		mapping.rotation.x = stream.read_float32();
		mapping.rotation.y = stream.read_float32();
		mapping.rotation.z = stream.read_float32();
		mapping.rotation.w = stream.read_float32();
		mappings.push_back(mapping);
	}
}

void bone_attachment_mapping_list::for_each(const std::function<bool(bone_attachment_mapping&)>& handler) {
	for (auto& mapping : mappings) {
		if (!handler(mapping)) {
			break;
		}
	}
}

void bone_attachment_mapping_list::remove_if(const std::function<bool(bone_attachment_mapping&)>& compare) {
	for (int i{ 0 }; i < static_cast<int>(mappings.size()); i++) {
		if (compare(mappings[i])) {
			mappings.erase(mappings.begin() + i);
			i--;
		}
	}
}

bool bone_attachment_mapping_list::exists(const bone_attachment_mapping& other) const {
	return std::any_of(mappings.begin(), mappings.end(), [&](const bone_attachment_mapping& mapping) {
		return mapping.is_same_mapping(other) && mapping.root_animation == other.root_animation;
	});
}

void bone_attachment_mapping_list::add(const bone_attachment_mapping& mapping) {
	if (!exists(mapping)) {
		mappings.push_back(mapping);
	}
}

bool bone_attachment_mapping_list::update(const model& root, int animation_index, const std::string& attachment_model, bone_attachment& attachment) const {
	if (animation_index < 0) {
		return false;
	}
	const auto& root_animation = root.animation(animation_index);
	for (const auto& mapping : mappings) {
		if (mapping.root_model != root.name() || mapping.root_animation != root_animation.name) {
			continue;
		}
		if (mapping.attached_model != attachment_model) {
			continue;
		}
		attachment.parent = mapping.attached_to_channel;
		if (attachment.parent != -1) {
			attachment.parent_bone = root.bone(root_animation.channels[attachment.parent].bone);
		}
		attachment.position = mapping.position;
		attachment.rotation = mapping.rotation;
		attachment.update();
		return true;
	}
	return false;
}

std::string bone_attachment_mapping_list::find_root_animation(const std::string& root_model, const std::string& attached_model, const std::string& attached_animation) const {
	for (const auto& mapping : mappings) {
		if (mapping.root_model != root_model) {
			continue;
		}
		if (mapping.attached_model != attached_model) {
			continue;
		}
		if (mapping.attached_animation != attached_animation) {
			continue;
		}
		return mapping.root_animation;
	}
	return {};
}

}
