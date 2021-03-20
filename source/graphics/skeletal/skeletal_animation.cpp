#include "graphics/skeletal/skeletal_animation.hpp"
#include "graphics/skeletal/skeletal_animation_update.hpp"

namespace nfwk {

skeletal_animation::skeletal_animation(int id) : id{ id } {

}

void skeletal_animation::apply_update(skeletal_animation_update& update) {
	is_attachment = update.is_attachment;
	root_transform = update.root_transform;
	attachment = update.attachment;
	reference = update.reference;
	if (update.reset) {
		play_timer.start();
		loops_completed = 0;
		loops_assigned = update.loops_if_reset;
		update.reset = false;
	}
}

}
