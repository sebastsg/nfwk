#include "graphics/skeletal/bone_attachment_mapping.hpp"

namespace no {

bool bone_attachment_mapping::is_same_mapping(const bone_attachment_mapping& that) const {
	return root_model == that.root_model && attached_model == that.attached_model && attached_animation == that.attached_animation;
}

std::string bone_attachment_mapping::mapping_string() const {
	return root_model + "." + root_animation + " -> " + attached_model + "." + attached_animation;
}

}
