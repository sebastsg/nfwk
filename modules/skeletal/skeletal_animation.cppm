module;

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

export module nfwk.skeletal:animation;

import std.core;
import nfwk.core;
export import :bone_attachment;

export namespace nfwk::skeletal {

class skeletal_animation_update {
public:

	bool is_attachment{ false };
	glm::mat4 root_transform{ 1.0f };
	bone_attachment attachment;
	int reference{ -1 };
	bool reset{ false };
	int loops_if_reset{ 0 };

};

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

	skeletal_animation(int id) : id{ id } {}

	void apply_update(skeletal_animation_update& update) {
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

};

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

	synced_skeletal_animation(const skeletal_animation& animation) {
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

};

}
