#pragma once

#include "graphics/skeletal/bone_attachment.hpp"

namespace no {

class skeletal_animation_update {
public:

	bool is_attachment{ false };
	glm::mat4 root_transform{ 1.0f };
	bone_attachment attachment;
	int reference{ -1 };
	bool reset{ false };
	int loops_if_reset{ 0 };

};

}
