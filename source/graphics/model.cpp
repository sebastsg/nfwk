#include "graphics/model.hpp"

namespace nfwk {

model::model(model&& that) noexcept : mesh{ std::move(that.mesh) } {
	std::swap(root_transform, that.root_transform);
	std::swap(min_vertex, that.min_vertex);
	std::swap(max_vertex, that.max_vertex);
	std::swap(bones, that.bones);
	std::swap(nodes, that.nodes);
	std::swap(animations, that.animations);
	std::swap(drawable, that.drawable);
	std::swap(texture, that.texture);
}

model& model::operator=(model&& that) noexcept {
	std::swap(mesh, that.mesh);
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

int model::index_of_animation(const std::u8string& name) const {
	for (std::size_t i{ 0 }; i < animations.size(); i++) {
		if (animations[i].name == name) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

int model::total_animations() const {
	return static_cast<int>(animations.size());
}

model_animation& model::animation(int index) {
	return animations[index];
}

const model_animation& model::animation(int index) const {
	return animations[index];
}

model_node& model::node(int index) {
	return nodes[index];
}

int model::total_nodes() const {
	return static_cast<int>(nodes.size());
}

glm::mat4 model::bone(int index) const {
	return bones[index];
}

void model::draw() const {
	mesh.draw();
}

bool model::is_drawable() const {
	return drawable && mesh.exists();
}

vector3f model::min() const {
	return min_vertex;
}

vector3f model::max() const {
	return max_vertex;
}

vector3f model::size() const {
	return max_vertex - min_vertex;
}

std::u8string model::texture_name() const {
	return texture;
}

std::u8string model::name() const {
	return model_name;
}

}