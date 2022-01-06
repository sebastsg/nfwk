#pragma once

#include "graphics/shader_variable.hpp"
#include "graphics/skeletal/bone_attachment.hpp"

#include <mutex>
#include <vector>

namespace nfwk {

class transform3;
class model;
class skeletal_animation;
class synced_skeletal_animation;
class skeletal_animation_update;
class shader;

class skeletal_animator {
public:

	struct {
		std::optional<shader_variable> bones;
	} shader_data;

	skeletal_animator(const model& skeleton);
	skeletal_animator(const skeletal_animator&) = delete;
	skeletal_animator(skeletal_animator&&) = delete;

	~skeletal_animator() = default;

	skeletal_animator& operator=(const skeletal_animator&) = delete;
	skeletal_animator& operator=(skeletal_animator&&) = delete;

	int add();
	void erase(int id);
	void set_transform(int id, const transform3& transform);
	void set_is_attachment(int id, bool attachment);
	void set_root_transform(int id, const glm::mat4& transform);
	int count() const;
	bool can_animate(int id) const;
	void reset(int id);
	bool will_be_reset(int id) const;
	glm::mat4 get_transform(int id, int transform) const;
	void set_attachment_bone(int id, const bone_attachment& attachment);
	bone_attachment get_attachment_bone(int id) const;

	void animate();
	void sync();

	void draw(shader& shader) const;
	void play(int id, int animation_index, int loops);
	void play(int id, std::string_view animation_name, int loops);

private:

	glm::mat4 next_interpolated_position(skeletal_animation& animation, int node) const;
	glm::mat4 next_interpolated_rotation(skeletal_animation& animation, int node) const;
	glm::mat4 next_interpolated_scale(skeletal_animation& animation, int node) const;
	void animate_node(skeletal_animation& animation, int parent, int node_index) const;
	void animate(skeletal_animation& animation) const;

	std::mutex animating;
	mutable std::mutex reading_synced;

	std::vector<transform3> model_transforms;
	std::vector<skeletal_animation> animations;
	std::vector<synced_skeletal_animation> synced_animations;
	std::vector<skeletal_animation_update> animation_updates;
	const model& skeleton;
	int active_count{ 0 };

};

}
