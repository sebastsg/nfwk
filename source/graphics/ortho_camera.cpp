#include "graphics/ortho_camera.hpp"

namespace no {

void ortho_camera::update() {
	if (target) {
		const vector2f goal{
			(target->position.x + target->scale.x / 2.0f) - width() / target_chase_aspect.x,
			(target->position.y + target->scale.y / 2.0f) - height() / target_chase_aspect.y
		};
		const vector2f delta{
			(goal.x - x()) * target_chase_speed.x,
			(goal.y - y()) * target_chase_speed.y
		};
		if (std::abs(delta.x) > 0.4f) {
			transform.position.x += delta.x;
		}
		if (std::abs(delta.y) > 0.4f) {
			transform.position.y += delta.y;
		}
	}
	transform.rotation = std::fmodf(transform.rotation, 360.0f);
	if (transform.rotation < 0.0f) {
		transform.rotation += 360.0f;
	}
}

float ortho_camera::x() const {
	return transform.position.x / zoom;
}

float ortho_camera::y() const {
	return transform.position.y / zoom;
}

vector2f ortho_camera::position() const {
	return transform.position / zoom;
}

float ortho_camera::width() const {
	return transform.scale.x / zoom;
}

float ortho_camera::height() const {
	return transform.scale.y / zoom;
}

vector2f ortho_camera::size() const {
	return transform.scale / zoom;
}

vector2f ortho_camera::mouse_position(const mouse& mouse) const {
	return (transform.position + mouse.position().to<float>()) / zoom;
}

glm::mat4 ortho_camera::rotation() const {
	return glm::rotate(glm::mat4{ 1.0f }, transform.rotation, glm::vec3{ 0.0f, 0.0f, 1.0f });
}

glm::mat4 ortho_camera::projection() const {
	return glm::ortho(0.0f, transform.scale.x, transform.scale.y, 0.0f, near_clipping_plane, far_clipping_plane);
}

glm::mat4 ortho_camera::view() const {
	vector2f rounded_position{ transform.position };
	rounded_position.ceil();
	const glm::vec3 negated_position{ -rounded_position.x, -rounded_position.y, -1.0f };
	const glm::mat4 matrix{ rotation() * glm::translate(glm::mat4{ 1.0f }, negated_position) };
	return glm::scale(matrix, { zoom, zoom, zoom });
}

glm::mat4 ortho_camera::view_projection() const {
	return view() * projection();
}

void ortho_camera::move_controller::update(ortho_camera& camera, const keyboard& keyboard) const {
	if (keyboard.is_key_down(left)) {
		camera.transform.position.x -= speed.x;
	}
	if (keyboard.is_key_down(up)) {
		camera.transform.position.y -= speed.y;
	}
	if (keyboard.is_key_down(down)) {
		camera.transform.position.y += speed.y;
	}
	if (keyboard.is_key_down(right)) {
		camera.transform.position.x += speed.x;
	}
}

}
