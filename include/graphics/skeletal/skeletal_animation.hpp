#pragma once

#include "graphics/skeletal/bone_attachment.hpp"
#include "timer.hpp"

namespace nfwk {

class skeletal_animation_update;

class skeletal_animation {
public:

	bool active{ true };
	int id{ -1 };
	int reference{ -1 };
	bone_attachment attachment;
	float time{ 0.0f };
	timer play_timer;
	glm::mat4 bones[48];
	glm::mat4 transforms[48];
	glm::mat4 root_transform{ 1.0f };
	bool is_attachment{ false };
	int bone_count{ 0 };
	int transform_count{ 0 };
	int loops_completed{ 0 };
	int loops_assigned{ 0 };
	int next_p{ 1 };
	int next_r{ 1 };
	int next_s{ 1 };

	skeletal_animation(int id);

	void apply_update(skeletal_animation_update& update);

};

}
