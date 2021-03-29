#pragma once

#include "vector3.hpp"
#include "glm/gtc/quaternion.hpp"

namespace nfwk {

struct bone_attachment_mapping {

	std::u8string root_model;
	std::u8string root_animation;
	std::u8string attached_model;
	std::u8string attached_animation;
	int attached_to_channel = -1;
	vector3f position;
	glm::quat rotation;

	bool is_same_mapping(const bone_attachment_mapping& that) const;
	std::u8string mapping_string() const;

};

}
