module;

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

export module nfwk.skeletal:bone_attachment_mapping;

import std.core;
import nfwk.core;

export namespace nfwk::skeletal {

class bone_attachment_mapping {
public:

	std::string root_model;
	std::string root_animation;
	std::string attached_model;
	std::string attached_animation;
	int attached_to_channel{ -1 };
	vector3f position;
	glm::quat rotation;

	bool is_same_mapping(const bone_attachment_mapping& that) const {
		return root_model == that.root_model && attached_model == that.attached_model && attached_animation == that.attached_animation;
	}

	std::string mapping_string() const {
		return root_model + "." + root_animation + " -> " + attached_model + "." + attached_animation;
	}

};

}
