#pragma once

#include "vector3.hpp"
#include "glm/gtc/quaternion.hpp"

#include <vector>

namespace nfwk {

struct animation_channel {

	using key_time = float;

	struct position_frame {
		key_time time{ 0.0f };
		vector3f position;

		position_frame() = default;
		position_frame(key_time time, vector3f position) : time{ time }, position{ position } {}

	};

	struct rotation_frame {
		key_time time{ 0.0f };
		glm::quat rotation;
		rotation_frame() = default;
		rotation_frame(key_time time, glm::quat rotation) : time{ time }, rotation{ rotation } {}
	};

	struct scale_frame {
		key_time time{ 0.0f };
		vector3f scale;
		scale_frame() = default;
		scale_frame(key_time time, vector3f scale) : time{ time }, scale{ scale } {}
	};

	std::vector<position_frame> positions;
	std::vector<rotation_frame> rotations;
	std::vector<scale_frame> scales;
	int bone{ -1 };

};

struct model_animation {
	std::u8string name;
	float duration{ 0.0f };
	float ticks_per_second{ 0.0f };
	std::vector<animation_channel> channels;
	std::vector<int> transitions;
};

struct model_node {
	std::u8string name;
	glm::mat4 transform;
	std::vector<int> children;
	int depth{ 0 };
};

}
