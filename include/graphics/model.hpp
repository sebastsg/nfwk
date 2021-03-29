#pragma once

#include "graphics/generic_vertex_array.hpp"
#include "graphics/model_data.hpp"
#include "graphics/model_animation.hpp"
#include "log.hpp"

namespace nfwk {

class model {
public:

	friend class model_instance;
	friend class skeletal_animator;

	model() = default;
	model(const model&) = delete;
	model(model&&) noexcept;

	~model() = default;

	model& operator=(const model&) = delete;
	model& operator=(model&&) noexcept;

	int index_of_animation(const std::u8string& name) const;
	int total_animations() const;
	model_animation& animation(int index);
	const model_animation& animation(int index) const;
	model_node& node(int index);
	int total_nodes() const;
	glm::mat4 bone(int index) const;

	template<typename Vertex, typename Index>
	void load(const model_data<Vertex, Index>& model) {
		if (model.shape.vertices.empty()) {
			warning(graphics::log, u8"Failed to load model");
			return;
		}
		mesh = { std::move(vertex_array<Vertex, Index>{model.shape.vertices, model.shape.indices }) };
		root_transform = model.transform;
		min_vertex = model.min;
		max_vertex = model.max;
		nodes = model.nodes;
		bones = model.bones;
		animations = model.animations;
		texture = model.texture;
		model_name = model.name;
		const auto vertices = model.shape.vertices.size();
		const auto indices = model.shape.indices.size();
		drawable = (vertices > 0 && indices > 0);
	}

	template<typename Vertex, typename Index>
	void load(const std::u8string& path) {
		model_data<Vertex, Index> model;
		import_model(path, model);
		if (model.shape.vertices.empty()) {
			warning(graphics::log, u8"Failed to load model: {}", path);
			return;
		}
		load(model);
	}

	void draw() const;

	bool is_drawable() const;

	vector3f min() const;
	vector3f max() const;
	vector3f size() const;

	std::u8string texture_name() const;
	std::u8string name() const;

private:

	generic_vertex_array mesh;
	glm::mat4 root_transform;
	std::vector<glm::mat4> bones;
	std::vector<model_node> nodes;
	std::vector<model_animation> animations;
	vector3f min_vertex;
	vector3f max_vertex;
	bool drawable{ false };
	std::u8string texture;
	std::u8string model_name;

};

}
