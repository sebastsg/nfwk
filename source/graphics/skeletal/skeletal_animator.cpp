#include "graphics/skeletal/skeletal_animator.hpp"
#include "graphics/skeletal/skeletal_animation.hpp"
#include "graphics/skeletal/skeletal_animation_update.hpp"
#include "graphics/skeletal/synced_skeletal_animation.hpp"
#include "graphics/model.hpp"
#include "graphics/shader.hpp"

namespace no {

static glm::mat4 interpolate_positions(float factor, const vector3f& begin, const vector3f& end) {
	const vector3f delta{ end - begin };
	const vector3f interpolated{ factor * delta };
	const vector3f translation{ begin + interpolated };
	return glm::translate(glm::mat4{ 1.0f }, { translation.x, translation.y, translation.z });
}

static glm::mat4 interpolate_rotations(float factor, const glm::quat& begin, const glm::quat& end) {
	return glm::mat4_cast(glm::normalize(glm::slerp(begin, end, factor)));
}

static glm::mat4 interpolate_scales(float factor, const vector3f& begin, const vector3f& end) {
	const vector3f delta{ end - begin };
	const vector3f interpolated{ factor * delta };
	const vector3f scale{ begin + interpolated };
	return glm::scale(glm::mat4{ 1.0f }, { scale.x, scale.y, scale.z });
}

skeletal_animator::skeletal_animator(const model& skeleton) : skeleton{ skeleton } {

}

void skeletal_animator::animate() {
	std::lock_guard lock{ animating };
	for (auto& animation : animations) {
		animate(animation);
	}
}

void skeletal_animator::sync() {
	std::lock_guard lock{ animating };
	std::lock_guard sync{ reading_synced };
	for (auto& animation : animations) {
		animation.apply_update(animation_updates[animation.id]);
		synced_animations[animation.id] = animation;
	}
}

int skeletal_animator::add() {
	std::lock_guard lock{ animating };
	for (int i{ 0 }; i < static_cast<int>(animations.size()); i++) {
		if (!animations[i].active) {
			animations[i] = { i };
			synced_animations[i] = animations[i];
			active_count++;
			return i;
		}
	}
	const int id{ static_cast<int>(animations.size()) };
	model_transforms.emplace_back();
	animations.emplace_back(id);
	synced_animations.emplace_back(id);
	animation_updates.emplace_back();
	active_count++;
	return id;
}

void skeletal_animator::erase(int id) {
	std::lock_guard lock{ animating };
	if (id >= 0 && id < static_cast<int>(animations.size())) {
		animations[id].active = false;
		synced_animations[id] = animations[id];
		active_count--;
	}
}

void skeletal_animator::set_transform(int id, const transform3& transform) {
	model_transforms[id] = transform;
}

void skeletal_animator::set_is_attachment(int id, bool attachment) {
	animation_updates[id].is_attachment = attachment;
}

void skeletal_animator::set_root_transform(int id, const glm::mat4& transform) {
	animation_updates[id].root_transform = transform;
}

glm::mat4 skeletal_animator::get_transform(int id, int transform) const {
	return synced_animations[id].transforms[transform];
}

void skeletal_animator::set_attachment_bone(int id, const bone_attachment& attachment) {
	animation_updates[id].attachment = attachment;
}

bone_attachment skeletal_animator::get_attachment_bone(int id) const {
	return synced_animations[id].attachment;
}

int skeletal_animator::count() const {
	return active_count;
}

bool skeletal_animator::can_animate(int id) const {
	std::lock_guard lock{ reading_synced };
	if (id < 0 || id >= static_cast<int>(synced_animations.size())) {
		return false;
	}
	const int reference{ synced_animations[id].reference };
	return reference >= 0 && reference < static_cast<int>(skeleton.animations.size());
}

void skeletal_animator::reset(int id) {
	std::lock_guard lock{ reading_synced };
	animation_updates[id].reset = true;
}

bool skeletal_animator::will_be_reset(int id) const {
	std::lock_guard lock{ reading_synced };
	if (synced_animations[id].reference < 0) {
		return false;
	}
	const auto& animation = skeleton.animations[synced_animations[id].reference];
	const double seconds{ static_cast<double>(synced_animations[id].played_for.milliseconds()) * 0.001 };
	const double play_duration{ seconds * static_cast<double>(animation.ticks_per_second) };
	return play_duration >= static_cast<double>(animation.duration);
}

void skeletal_animator::draw() const {
	std::lock_guard lock{ reading_synced };
	skeleton.bind();
	for (auto& animation : synced_animations) {
		if (animation.active) {
			set_shader_model(model_transforms[animation.id]);
			shader.bones.set(animation.bones, animation.bone_count);
			skeleton.draw();
		}
	}
}

void skeletal_animator::play(int id, int animation_index, int loops) {
	std::lock_guard lock{ reading_synced };
	auto& animation = animation_updates[id];
	if (animation.reference != animation_index) {
		animation.reset = true;
		animation.loops_if_reset = loops;
		animation.reference = animation_index;
	}
}

void skeletal_animator::play(int id, const std::string& animation_name, int loops) {
	play(id, skeleton.index_of_animation(animation_name), loops);
}

glm::mat4 skeletal_animator::next_interpolated_position(skeletal_animation& animation, int node_index) const {
	const auto& node = skeleton.animations[animation.reference].channels[node_index];
	for (int p{ animation.next_p }; p < static_cast<int>(node.positions.size()); p++) {
		if (node.positions[p].time > animation.time) {
			animation.next_p = p;
			const auto& current = node.positions[p - 1];
			const auto& next = node.positions[p];
			const float delta_time{ next.time - current.time };
			const float factor{ (animation.time - current.time) / delta_time };
			return interpolate_positions(factor, current.position, next.position);
		}
	}
	if (node.positions.empty()) {
		return glm::mat4{ 1.0f };
	}
	animation.next_p = 1;
	const auto& translation{ node.positions.front().position };
	return glm::translate(glm::mat4{ 1.0f }, { translation.x, translation.y, translation.z });
}

glm::mat4 skeletal_animator::next_interpolated_rotation(skeletal_animation& animation, int node_index) const {
	const auto& node = skeleton.animations[animation.reference].channels[node_index];
	for (int r{ animation.next_r }; r < static_cast<int>(node.rotations.size()); r++) {
		if (node.rotations[r].time > animation.time) {
			animation.next_r = r;
			const auto& current = node.rotations[r - 1];
			const auto& next = node.rotations[r];
			const float delta_time{ next.time - current.time };
			const float factor{ (animation.time - current.time) / delta_time };
			return interpolate_rotations(factor, current.rotation, next.rotation);
		}
	}
	if (node.rotations.empty()) {
		return glm::mat4{ 1.0f };
	}
	animation.next_r = 1;
	return glm::mat4_cast(node.rotations.front().rotation);
}

glm::mat4 skeletal_animator::next_interpolated_scale(skeletal_animation& animation, int node_index) const {
	const auto& node = skeleton.animations[animation.reference].channels[node_index];
	for (int s{ animation.next_s }; s < static_cast<int>(node.scales.size()); s++) {
		if (node.scales[s].time > animation.time) {
			animation.next_s = s;
			const auto& current = node.scales[s - 1];
			const auto& next = node.scales[s];
			const float delta_time{ next.time - current.time };
			const float factor{ (animation.time - current.time) / delta_time };
			return interpolate_scales(factor, current.scale, next.scale);
		}
	}
	if (node.scales.empty()) {
		return glm::mat4{ 1.0f };
	}
	animation.next_s = 1;
	const auto& scale = node.scales.front().scale;
	return glm::scale(glm::mat4{ 1.0f }, { scale.x, scale.y, scale.z });
}

void skeletal_animator::animate_node(skeletal_animation& animation, int parent, int node_index) const {
	const auto& reference = skeleton.animations[animation.reference];
	glm::mat4 node_transform{ skeleton.nodes[node_index].transform };
	const auto& node = reference.channels[node_index];
	if (node.positions.size() > 0 || node.rotations.size() > 0 || node.scales.size() > 0) {
		const glm::mat4 translation{ next_interpolated_position(animation, node_index) };
		const glm::mat4 rotation{ next_interpolated_rotation(animation, node_index) };
		const glm::mat4 scale{ next_interpolated_scale(animation, node_index) };
		node_transform = translation * rotation * scale;
	}
	animation.transforms[node_index] = animation.transforms[parent] * node_transform;
	if (node.bone != -1) {
		if (animation.is_attachment) {
			animation.bones[node.bone] = animation.transforms[node_index] * animation.attachment.parent_bone * animation.attachment.child_bone * skeleton.root_transform;
		} else {
			animation.bones[node.bone] = skeleton.root_transform * animation.transforms[node_index] * skeleton.bones[node.bone];
		}
	}
	for (const int child : skeleton.nodes[node_index].children) {
		animate_node(animation, node_index, child);
	}
}

void skeletal_animator::animate(skeletal_animation& animation) const {
	if (!animation.active || animation.reference == -1) {
		return;
	}
	const auto& reference = skeleton.animations[animation.reference];
	const double seconds{ static_cast<double>(animation.play_timer.milliseconds()) * 0.001 };
	const double play_duration{ seconds * static_cast<double>(reference.ticks_per_second) };
	const float time{ animation.time };
	animation.time = static_cast<float>(std::fmod(play_duration, static_cast<double>(reference.duration)));
	if (time > animation.time) {
		animation.next_p = 1;
		animation.next_r = 1;
		animation.next_s = 1;
	}
	animation.transforms[0] = animation.root_transform;
	animation.bone_count = static_cast<int>(skeleton.bones.size());
	animation.transform_count = skeleton.total_nodes();
	animate_node(animation, 0, 0);
}

}
