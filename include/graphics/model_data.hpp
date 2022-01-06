#pragma once

#include "transform.hpp"
#include "graphics/vertex_array_data.hpp"
#include "graphics/vertex.hpp"
#include "graphics/model_animation.hpp"
#include "io.hpp"
#include "log.hpp"

#include <functional>

namespace nfwk {

template<typename Vertex, typename Index>
class model_data {
public:

	glm::mat4 transform;
	vector3f min;
	vector3f max;
	vertex_array_data<Vertex, Index> shape;
	std::vector<std::string> bone_names;
	std::vector<glm::mat4> bones;
	std::vector<model_node> nodes;
	std::vector<model_animation> animations;
	std::string texture;
	std::string name;

	template<typename CustomVertex>
	model_data<CustomVertex, Index> to(const std::function<CustomVertex(const Vertex&)>& mapper) const {
		model_data<CustomVertex, Index> that;
		that.transform = transform;
		that.min = min;
		that.max = max;
		for (const auto& vertex : shape.vertices) {
			that.shape.vertices.push_back(mapper(vertex));
		}
		that.shape.indices = shape.indices;
		that.bone_names = bone_names;
		that.bones = bones;
		that.nodes = nodes;
		that.animations = animations;
		that.texture = texture;
		that.name = name;
		return that;
	}

};

template<typename Vertex, typename Index>
model_data<Vertex, Index> create_box_model_data(const std::function<Vertex(const vector3f&)>& mapper) {
	model_data<Vertex, Index> data;
	data.name = "box";
	data.min = 0.0f;
	data.max = 1.0f;
	constexpr vector3f vertices[]{
		{ 0.0f, 0.0f, 0.0f }, // (top left) lower - 0
		{ 0.0f, 1.0f, 0.0f }, // (top left) upper - 1
		{ 1.0f, 0.0f, 0.0f }, // (top right) lower - 2
		{ 1.0f, 1.0f, 0.0f }, // (top right) upper - 3
		{ 0.0f, 0.0f, 1.0f }, // (bottom left) lower - 4
		{ 0.0f, 1.0f, 1.0f }, // (bottom left) upper - 5
		{ 1.0f, 0.0f, 1.0f }, // (bottom right) lower - 6
		{ 1.0f, 1.0f, 1.0f }, // (bottom right) upper - 7
	};
	for (const auto& vertex : vertices) {
		data.shape.vertices.push_back(mapper(vertex));
	}
	data.shape.indices = {
		0, 1, 5, 5, 4, 0, // left
		0, 1, 3, 3, 2, 0, // back
		2, 3, 7, 7, 6, 2, // right
		5, 7, 6, 6, 4, 5, // front
		0, 2, 6, 6, 4, 0, // bottom
		1, 3, 7, 7, 5, 1, // top
	};
	return data;
}

struct model_import_options {
	struct {
		bool create_default{ false };
		std::string bone_name{ "Bone" };
	} bones;
};

struct model_conversion_options {
	model_import_options import;
	std::function<void(const std::string&, const model_data<animated_mesh_vertex, int>&)> exporter;
};

template<typename Vertex, typename Index>
void export_model(const std::string& path, const model_data<Vertex, Index>& model) {
	io_stream stream;
	stream.write_struct(model.transform);
	stream.write_struct(model.min);
	stream.write_struct(model.max);
	stream.write_string(model.texture);
	stream.write_string(model.name);
	stream.write_size(sizeof(Vertex));
	stream.write_array<Vertex>(model.shape.vertices);
	stream.write_array<Index>(model.shape.indices);
	stream.write_string_array(model.bone_names);
	stream.write_array<glm::mat4>(model.bones);
	stream.write_size<size_length::two_bytes>(model.nodes.size());
	for (const auto& node : model.nodes) {
		stream.write_string(node.name);
		stream.write_struct(node.transform);
		stream.write_array<std::int16_t, int>(node.children);
	}
	stream.write_size(model.animations.size(), size_length::two_bytes);
	for (const auto& animation : model.animations) {
		stream.write_string(animation.name);
		stream.write_float32(animation.duration);
		stream.write_float32(animation.ticks_per_second);
		stream.write_size<size_length::two_bytes>(animation.channels.size());
		for (const auto& node : animation.channels) {
			stream.write_int16(static_cast<std::int16_t>(node.bone));
			stream.write_size<size_length::two_bytes>(node.positions.size());
			for (const auto& position : node.positions) {
				stream.write_float32(position.time);
				stream.write_struct(position.position);
			}
			stream.write_size<size_length::two_bytes>(node.rotations.size());
			for (const auto& rotation : node.rotations) {
				stream.write_float32(rotation.time);
				stream.write_struct(rotation.rotation);
			}
			stream.write_size<size_length::two_bytes>(node.scales.size());
			for (const auto& scale : node.scales) {
				stream.write_float32(scale.time);
				stream.write_struct(scale.scale);
			}
		}
		stream.write_array<std::int16_t, int>(animation.transitions);
	}
	write_file(path, stream);
}

template<typename Vertex, typename Index>
void import_model(const std::string& path, model_data<Vertex, Index>& model) {
	io_stream stream;
	read_file(path, stream);
	if (stream.write_index() == 0) {
		warning(graphics::log, "Failed to open file: {}", path);
		return;
	}
	model.transform = stream.read_struct<glm::mat4>();
	model.min = stream.read_struct<vector3f>();
	model.max = stream.read_struct<vector3f>();
	model.texture = stream.read_string();
	model.name = stream.read_string();
	const auto vertex_size = stream.read_size();
	if (vertex_size != sizeof(Vertex)) {
		warning(graphics::log, "{} != {}. File: {}", vertex_size, sizeof(Vertex), path);
		return;
	}
	model.shape.vertices = stream.read_array<Vertex>();
	model.shape.indices = stream.read_array<Index>();
	model.bone_names = stream.read_string_array();
	model.bones = stream.read_array<glm::mat4>();
	const auto node_count = stream.read_size<size_length::two_bytes>();
	for (std::size_t n{ 0 }; n < node_count; n++) {
		auto& node = model.nodes.emplace_back();
		node.name = stream.read_string();
		node.transform = stream.read_struct<glm::mat4>();
		node.children = stream.read_array<int, std::int16_t>();
	}
	const auto animation_count = stream.read_size<size_length::two_bytes>();
	for (std::size_t a{ 0 }; a < animation_count; a++) {
		auto& animation{ model.animations.emplace_back() };
		animation.name = stream.read_string();
		animation.duration = stream.read_float32();
		animation.ticks_per_second = stream.read_float32();
		const auto animation_node_count = stream.read_size<size_length::two_bytes>();
		for (std::size_t n{ 0 }; n < animation_node_count; n++) {
			auto& node = animation.channels.emplace_back();
			node.bone = static_cast<int>(stream.read_int16());
			const auto position_count = stream.read_size<size_length::two_bytes>();
			for (std::size_t p{ 0 }; p < position_count; p++) {
				auto& position = node.positions.emplace_back();
				position.time = stream.read_float32();
				position.position = stream.read_struct<vector3f>();
			}
			const auto rotation_count = stream.read_size<size_length::two_bytes>();
			for (std::size_t r{ 0 }; r < rotation_count; r++) {
				auto& rotation = node.rotations.emplace_back();
				rotation.time = stream.read_float32();
				rotation.rotation = stream.read_struct<glm::quat>();
			}
			const auto scale_count = stream.read_size<size_length::two_bytes>();
			for (std::size_t s{ 0 }; s < scale_count; s++) {
				auto& scale = node.scales.emplace_back();
				scale.time = stream.read_float32();
				scale.scale = stream.read_struct<vector3f>();
			}
		}
		animation.transitions = stream.read_array<int, std::int16_t>();
	}
}

// if multiple models have identical vertex data, they can be merged into one model with all animations
// source files must already be converted to nom format. validation is done during the merging process.
template<typename Vertex, typename Index>
model_data<Vertex, Index> merge_model_animations(const std::vector<model_data<Vertex, Index>>& models) {
	if (models.empty()) {
		return {};
	}
	if (models.size() == 1) {
		return models.front();
	}
	model_data<Vertex, Index> output = models.front();
	for (std::size_t m{ 1 }; m < models.size(); m++) {
		auto& model = models[m];
		if (model.transform != output.transform) {
			warning(graphics::log, "Root transform not identical. Not skipped.");
		}
		if (output.nodes.size() != model.nodes.size()) {
			warning(graphics::log, "Different number of node transforms. Skipping.");
			continue;
		}
		bool equal{ true };
		for (std::size_t n{ 0 }; n < output.nodes.size(); n++) {
			// maybe this isn't that important?
			/*if (output.nodes[n].transform != model.nodes[n].transform) {
				WARNING("Different node transform " << n << ". Skipping.");
				equal = false;
				break;
			}*/
			if (output.nodes[n].children != model.nodes[n].children) {
				warning(graphics::log, "Different node children {}. Skipping.", n);
				equal = false;
				break;
			}
		}
		if (!equal) {
			continue;
		}
		if (model.bones.size() != output.bones.size()) {
			warning(graphics::log, "Mesh not identical. Skipping.");
			//continue;
		}
		for (std::size_t i = 0; i < model.bones.size(); i++) {
			if (model.bones[i] != output.bones[i]) {
				warning(graphics::log, "Mesh not identical. Skipping.");
				//continue;
			}
		}
		if (model.shape != output.shape) {
			warning(graphics::log, "Mesh not identical. Skipping.");
			//continue;
		}
		for (auto& animation : model.animations) {
			bool skip{ false };
			for (auto& existing_animation : output.animations) {
				if (animation.name == existing_animation.name) {
					warning(graphics::log, "{} already exists. Skipping animation.", animation.name);
					skip = true;
					continue;
				}
			}
			if (skip) {
				continue;
			}
			output.animations.push_back(animation);
		}
	}
	std::vector<int> default_transitions;
	default_transitions.insert(default_transitions.begin(), output.animations.size(), 0);
	for (auto& animation : output.animations) {
		animation.transitions = default_transitions;
	}
	output.name = "";
	return output;
}


#if 0
void convert_model(const std::string& source, const std::string& destination, model_conversion_options options);
#endif

transform3 load_model_bounding_box(const std::filesystem::path& path);

}
