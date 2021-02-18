module;

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

export module nfwk.skeletal:bone_attachment_mapping_list;

import std.core;
import nfwk.core;
import nfwk.graphics;
export import :bone_attachment_mapping;

#if 0
export namespace nfwk::skeletal {

class bone_attachment_mapping_list {
public:

	void save(const std::string& path) {
		io_stream stream;
		stream.write(static_cast<std::int32_t>(mappings.size()));
		for (const auto& attachment : mappings) {
			stream.write(attachment.root_model);
			stream.write(attachment.root_animation);
			stream.write(attachment.attached_model);
			stream.write(attachment.attached_animation);
			stream.write(static_cast<std::int32_t>(attachment.attached_to_channel));
			stream.write(attachment.position);
			stream.write(attachment.rotation);
		}
		write_file(path, stream);
	}

	void load(const std::string& path) {
		if (io_stream stream{ path }; !stream.empty()) {
			const auto count = stream.read<std::int32_t>();
			for (std::int32_t i{ 0 }; i < count; i++) {
				bone_attachment_mapping mapping;
				mapping.root_model = stream.read<std::string>();
				mapping.root_animation = stream.read<std::string>();
				mapping.attached_model = stream.read<std::string>();
				mapping.attached_animation = stream.read<std::string>();
				mapping.attached_to_channel = stream.read<std::int32_t>();
				mapping.position = stream.read<vector3f>();
				mapping.rotation = stream.read<glm::quat>();
				mappings.push_back(mapping);
			}
		}
	}

	void for_each(const std::function<bool(bone_attachment_mapping&)>& handler) {
		for (auto& mapping : mappings) {
			if (!handler(mapping)) {
				break;
			}
		}
	}

	void remove_if(const std::function<bool(bone_attachment_mapping&)>& compare) {
		for (int i{ 0 }; i < static_cast<int>(mappings.size()); i++) {
			if (compare(mappings[i])) {
				mappings.erase(mappings.begin() + i);
				i--;
			}
		}
	}

	bool exists(const bone_attachment_mapping& mapping) const {
		for (const auto& mapping : mappings) {
			if (mapping.is_same_mapping(other) && mapping.root_animation == other.root_animation) {
				return true;
			}
		}
		return false;
	}

	void add(const bone_attachment_mapping& mapping) {
		if (!exists(mapping)) {
			mappings.push_back(mapping);
		}
	}

	bool update(const model& root, int animation_index, const std::string& attachment_model, bone_attachment& attachment) const {
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

	std::string find_root_animation(const std::string& root_model, const std::string& attached_model, const std::string& attached_animation) const {
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
		return "";
	}

private:

	std::vector<bone_attachment_mapping> mappings;

};

}
#endif