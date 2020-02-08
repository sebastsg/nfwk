#include "grid.hpp"
#include "draw.hpp"
#include "camera.hpp"
#include "surface.hpp"

namespace no {

static std::unique_ptr<rectangle> grid;
static int grid_texture{ 0 };
static bool own_texture{ false };

void create_grid(std::optional<int> texture) {
	grid = std::make_unique<rectangle>();
	if (texture.has_value()) {
		grid_texture = texture.value();
		own_texture = false;
	} else {
		grid_texture = create_texture(surface{ 2, 2, pixel_format::rgba, 0x22ffffff });
		own_texture = true;
	}
}

void destroy_grid() {
	grid.reset();
	if (own_texture) {
		delete_texture(grid_texture);
	}
}

void draw_grid(const ortho_camera& camera, vector2f size) {
	bind_texture(grid_texture);
	transform2 transform;
	transform.scale = { camera.width(), 1.0f };
	transform.position.x = camera.x() - std::fmodf(camera.x(), size.x);
	grid->bind();
	for (float y{ 0.0f }; y < camera.height(); y += size.y) {
		transform.position.y = camera.y() - std::fmodf(camera.y(), size.y) + static_cast<float>(y);
		no::set_shader_model(transform);
		grid->draw();
	}
	transform.scale = { 1.0f, camera.height() };
	transform.position.y = camera.y() - std::fmodf(camera.y(), size.y);
	for (float x{ 0.0f }; x < camera.width(); x += size.x) {
		transform.position.x = camera.x() - std::fmodf(camera.x(), size.x) + static_cast<float>(x);
		no::set_shader_model(transform);
		grid->draw();
	}
}

}
