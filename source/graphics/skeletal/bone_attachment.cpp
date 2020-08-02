#include "graphics/skeletal/bone_attachment.hpp"

namespace no {

void bone_attachment::update() {
	const glm::mat4 translate{ glm::translate(glm::mat4{ 1.0f }, { position.x, position.y, position.z }) };
	const glm::mat4 rotate{ glm::mat4_cast(glm::normalize(rotation)) };
	child_bone = translate * rotate;
}

}
