module;

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

export module nfwk.skeletal:bone_attachment;

import std.core;
import nfwk.core;

export namespace nfwk::skeletal {

class bone_attachment {
public:

	int parent{ -1 };
	glm::mat4 parent_bone;
	glm::mat4 child_bone;
	vector3f position;
	glm::quat rotation;

	void update() {
		const glm::mat4 translate{ glm::translate(glm::mat4{ 1.0f }, { position.x, position.y, position.z }) };
		const glm::mat4 rotate{ glm::mat4_cast(glm::normalize(rotation)) };
		child_bone = translate * rotate;
	}

};

}
