#include "graphics/sky.hpp"
#include "graphics/perspective_camera.hpp"
#include "graphics/vertex.hpp"
#include "graphics/model.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"
#include "graphics/draw.hpp"

namespace nfwk {

struct skybox_vertex {
	static constexpr vertex_attribute_specification attributes[]{ 3, 2 };
	vector3f position;
	vector2f tex_coords;
};

std::unique_ptr<model> skybox;

void create_skybox() {
	model_data<skybox_vertex, unsigned short> data;
	data.name = u8"skybox";
	data.min = 0.0f;
	data.max = 1.0f;
	data.shape.vertices = {
		// left - 0, 1, 3, 3, 2, 0
		{{ 0.0f, 0.0f, 0.0f }, { 1.0f / 4.0f, 2.0f / 3.0f }}, // (top left) lower - 0
		{{ 0.0f, 1.0f, 0.0f }, { 1.0f / 4.0f, 1.0f / 3.0f }}, // (top left) upper - 1
		{{ 0.0f, 0.0f, 1.0f }, { 0.0f / 4.0f, 2.0f / 3.0f }}, // (bottom left) lower - 4
		{{ 0.0f, 1.0f, 1.0f }, { 0.0f / 4.0f, 1.0f / 3.0f }}, // (bottom left) upper - 5

		// front - 4, 5, 7, 7, 6, 0
		{{ 0.0f, 0.0f, 0.0f }, { 1.0f / 4.0f, 2.0f / 3.0f }}, // (top left) lower - 0
		{{ 0.0f, 1.0f, 0.0f }, { 1.0f / 4.0f, 1.0f / 3.0f }}, // (top left) upper - 1
		{{ 1.0f, 0.0f, 0.0f }, { 2.0f / 4.0f, 2.0f / 3.0f }}, // (top right) lower - 2
		{{ 1.0f, 1.0f, 0.0f }, { 2.0f / 4.0f, 1.0f / 3.0f }}, // (top right) upper - 3

		// right - 8, 9, 11, 11, 10, 8
		{{ 1.0f, 0.0f, 0.0f }, { 2.0f / 4.0f, 2.0f / 3.0f }}, // (top right) lower - 2
		{{ 1.0f, 1.0f, 0.0f }, { 2.0f / 4.0f, 1.0f / 3.0f }}, // (top right) upper - 3
		{{ 1.0f, 0.0f, 1.0f }, { 3.0f / 4.0f, 2.0f / 3.0f }}, // (bottom right) lower - 6
		{{ 1.0f, 1.0f, 1.0f }, { 3.0f / 4.0f, 1.0f / 3.0f }}, // (bottom right) upper - 7

		// back - 12, 13, 15, 15, 14, 12
		{{ 0.0f, 0.0f, 1.0f }, { 4.0f / 4.0f, 2.0f / 3.0f }}, // (bottom left) lower - 4
		{{ 0.0f, 1.0f, 1.0f }, { 4.0f / 4.0f, 1.0f / 3.0f }}, // (bottom left) upper - 5
		{{ 1.0f, 0.0f, 1.0f }, { 3.0f / 4.0f, 2.0f / 3.0f }}, // (bottom right) lower - 6
		{{ 1.0f, 1.0f, 1.0f }, { 3.0f / 4.0f, 1.0f / 3.0f }}, // (bottom right) upper - 7

		// bottom - 16, 17, 19, 19, 18, 16
		{{ 0.0f, 0.0f, 0.0f }, { 1.0f / 4.0f, 2.0f / 3.0f }}, // (top left) lower - 0
		{{ 1.0f, 0.0f, 0.0f }, { 2.0f / 4.0f, 2.0f / 3.0f }}, // (top right) lower - 2
		{{ 0.0f, 0.0f, 1.0f }, { 1.0f / 4.0f, 3.0f / 3.0f }}, // (bottom left) lower - 4
		{{ 1.0f, 0.0f, 1.0f }, { 2.0f / 4.0f, 3.0f / 3.0f }}, // (bottom right) lower - 6

		// top - 20, 21, 23, 23, 22, 20
		{{ 0.0f, 1.0f, 0.0f }, { 1.0f / 4.0f, 1.0f / 3.0f }}, // (top left) upper - 1
		{{ 1.0f, 1.0f, 0.0f }, { 2.0f / 4.0f, 1.0f / 3.0f }}, // (top right) upper - 3
		{{ 0.0f, 1.0f, 1.0f }, { 1.0f / 4.0f, 2.0f / 3.0f }}, // (bottom left) upper - 5
		{{ 1.0f, 1.0f, 1.0f }, { 2.0f / 4.0f, 2.0f / 3.0f }}, // (bottom right) upper - 7
	};
	data.shape.indices = {
		0,  1,  3,  3,  2,  0,  // left
		4,  5,  7,  7,  6,  0,  // front
		8,  9,  11, 11, 10, 8,  // right
		12, 13, 15, 15, 14, 12, // back
		16, 17, 19, 19, 18, 16, // bottom
		20, 21, 23, 23, 22, 20, // top
	};
	skybox = std::make_unique<model>();
	skybox->load(data);
}

void delete_skybox() {
	skybox = nullptr;
}

void draw_skybox(const perspective_camera& camera, float size, shader& shader, const texture& texture) {
	shader.set_view_projection(camera);
	texture.bind();
	shader.draw(*skybox, transform3{ camera.transform.position - size / 2.0f - camera.transform.scale / 2.0f, {}, size });
}

}
