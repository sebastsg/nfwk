#pragma once

#include "platform.hpp"

#if ENABLE_GRAPHICS

#include "transform.hpp"
#include "input.hpp"

namespace no {

class ortho_camera {
public:

	class move_controller;

	transform2* target{ nullptr };
	transform2 transform;
	float zoom{ 1.0f };

	float near_clipping_plane{ 0.01f };
	float far_clipping_plane{ 1000.0f };

	vector2f target_chase_speed{ 0.05f, 0.05f };
	vector2f target_chase_aspect{ 2.0f, 1.5f };

	void update();

	// from screen to world coordinates (divided by zoom)
	float x() const;
	float y() const;
	vector2f position() const;
	float width() const;
	float height() const;
	vector2f size() const;
	vector2f mouse_position(const mouse& mouse) const;

	glm::mat4 rotation() const;
	glm::mat4 view() const;
	glm::mat4 projection() const;
	glm::mat4 view_projection() const;

};

class ortho_camera::move_controller {
public:

	vector2f speed{ 16.0f };

	key up{ key::w };
	key left{ key::a };
	key down{ key::s };
	key right{ key::d };

	void update(ortho_camera& camera, const keyboard& keyboard) const;

};


}

#endif
