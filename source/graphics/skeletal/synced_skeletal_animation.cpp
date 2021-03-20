#include "graphics/skeletal/synced_skeletal_animation.hpp"
#include "graphics/skeletal/skeletal_animation.hpp"

namespace nfwk {

synced_skeletal_animation::synced_skeletal_animation(const skeletal_animation& animation) {
	active = animation.active;
	id = animation.id;
	reference = animation.reference;
	attachment = animation.attachment;
	std::copy(std::begin(animation.bones), std::end(animation.bones), std::begin(bones));
	std::copy(std::begin(animation.transforms), std::end(animation.transforms), std::begin(transforms));
	root_transform = animation.root_transform;
	bone_count = animation.bone_count;
	transform_count = animation.transform_count;
	played_for = animation.play_timer;
}

}
