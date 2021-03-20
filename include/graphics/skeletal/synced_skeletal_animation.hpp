#pragma once

#include "graphics/skeletal/bone_attachment.hpp"
#include "timer.hpp"

namespace nfwk {

class skeletal_animation;

class synced_skeletal_animation {
public:

	bool active{ true };
	int id{ -1 };
	int reference{ -1 };
	bone_attachment attachment;
	glm::mat4 bones[48];
	glm::mat4 transforms[48];
	glm::mat4 root_transform{ 1.0f };
	int bone_count{ 0 };
	int transform_count{ 0 };
	timer played_for;

	synced_skeletal_animation(const skeletal_animation& animation);

};

}
