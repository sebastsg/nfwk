#pragma once

#include "vector3.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

namespace nfwk {

class bone_attachment {
public:

	int parent{ -1 };
	glm::mat4 parent_bone;
	glm::mat4 child_bone;
	vector3f position;
	glm::quat rotation;

	void update();

};

}
