#include "graphics/grid.hpp"
#include "graphics/ortho_camera.hpp"
#include "graphics/surface.hpp"
#include "graphics/texture.hpp"
#include "graphics/shader.hpp"

namespace nfwk {

grid::grid(std::shared_ptr<texture> texture_) {
	grid_texture = texture_ ? texture_ : std::make_shared<texture>(surface{ 2, 2, pixel_format::rgba, 0x22ffffff });
}

grid::~grid() {

}

void grid::draw(shader& shader, const ortho_camera& camera, vector2f size) {
	grid_texture->bind();
	transform2 transform;
	transform.scale = { camera.width(), 1.0f };
	transform.position.x = camera.x() - std::fmodf(camera.x(), size.x);
	for (float y{ 0.0f }; y < camera.height(); y += size.y) {
		transform.position.y = camera.y() - std::fmodf(camera.y(), size.y) + static_cast<float>(y);
		shader.set_model(transform);
		shape.draw();
	}
	transform.scale = { 1.0f, camera.height() };
	transform.position.y = camera.y() - std::fmodf(camera.y(), size.y);
	for (float x{ 0.0f }; x < camera.width(); x += size.x) {
		transform.position.x = camera.x() - std::fmodf(camera.x(), size.x) + static_cast<float>(x);
		shader.set_model(transform);
		shape.draw();
	}
}

}
