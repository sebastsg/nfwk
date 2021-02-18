module;

#include <glm/gtc/matrix_transform.hpp>

export module nfwk.graphics:model;

import std.core;
import nfwk.core;
import :model_animation;
import :model_data;

namespace nfwk::skeletal {
class skeletal_animator;
}

export namespace nfwk {

class model {
public:

	friend class model_instance;
	friend class skeletal::skeletal_animator;

	model() = default;

	model(const model&) = delete;

	model(model&& that) noexcept {// : mesh{ std::move(that.mesh) } {
		std::swap(root_transform, that.root_transform);
		std::swap(min_vertex, that.min_vertex);
		std::swap(max_vertex, that.max_vertex);
		std::swap(bones, that.bones);
		std::swap(nodes, that.nodes);
		std::swap(animations, that.animations);
		std::swap(drawable, that.drawable);
		std::swap(texture, that.texture);
	}

	~model() = default;

	model& operator=(const model&) = delete;

	model& operator=(model&& that) noexcept {
		//std::swap(mesh, that.mesh);
		std::swap(root_transform, that.root_transform);
		std::swap(min_vertex, that.min_vertex);
		std::swap(max_vertex, that.max_vertex);
		std::swap(bones, that.bones);
		std::swap(nodes, that.nodes);
		std::swap(animations, that.animations);
		std::swap(drawable, that.drawable);
		std::swap(texture, that.texture);
		return *this;
	}

	int index_of_animation(const std::string& name) const {
		for (size_t i{ 0 }; i < animations.size(); i++) {
			if (animations[i].name == name) {
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	int total_animations() const {
		return static_cast<int>(animations.size());
	}

	model_animation& animation(int index) {
		return animations[index];
	}

	const model_animation& animation(int index) const {
		return animations[index];
	}

	model_node& node(int index) {
		return nodes[index];
	}

	int total_nodes() const {
		return static_cast<int>(nodes.size());
	}

	glm::mat4 bone(int index) const {
		return bones[index];
	}

	template<typename Vertex, typename Index>
	void load(const model_data<Vertex, Index>& model) {
		if (model.shape.vertices.empty()) {
			warning("graphics", "Failed to load model");
			return;
		}
		//mesh = { std::move(vertex_array<Vertex, Index>{model.shape.vertices, model.shape.indices }) };
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
	void load(const std::string& path) {
		//model_data<Vertex, Index> model;
		//import_model(path, model);
		//if (model.shape.vertices.empty()) {
		//	warning("graphics", "Failed to load model: {}", path);
		//	return;
		//}
		//load(model);
	}

	void bind() const {
		//mesh.bind();
	}

	void draw() const {
		//mesh.draw();
	}

	bool is_drawable() const {
		return drawable;//&& mesh.exists();
	}

	vector3f min() const {
		return min_vertex;
	}

	vector3f max() const {
		return max_vertex;
	}

	vector3f size() const {
		return max_vertex - min_vertex;
	}

	std::string texture_name() const {
		return texture;
	}

	std::string name() const {
		return model_name;
	}

private:

	//generic_vertex_array mesh;
	glm::mat4 root_transform;
	std::vector<glm::mat4> bones;
	std::vector<model_node> nodes;
	std::vector<model_animation> animations;
	vector3f min_vertex;
	vector3f max_vertex;
	bool drawable{ false };
	std::string texture;
	std::string model_name;

};

}
