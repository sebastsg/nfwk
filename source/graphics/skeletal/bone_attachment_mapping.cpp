#include "graphics/skeletal/bone_attachment_mapping.hpp"

namespace nfwk {

bool bone_attachment_mapping::is_same_mapping(const bone_attachment_mapping& that) const {
	return root_model == that.root_model && attached_model == that.attached_model && attached_animation == that.attached_animation;
}

std::u8string bone_attachment_mapping::mapping_string() const {
	return root_model + u8"." + root_animation + u8" -> " + attached_model + u8"." + attached_animation;
}

}
