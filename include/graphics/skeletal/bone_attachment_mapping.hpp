#pragma once

#include "vector3.hpp"
#include "glm/gtc/quaternion.hpp"

namespace no {

struct bone_attachment_mapping {

	std::string root_model;
	std::string root_animation;
	std::string attached_model;
	std::string attached_animation;
	int attached_to_channel = -1;
	vector3f position;
	glm::quat rotation;

	bool is_same_mapping(const bone_attachment_mapping& that) const;
	std::string mapping_string() const;

};

}
