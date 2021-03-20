#include "graphics/sprite_animation.hpp"
#include "graphics/texture.hpp"
#include "graphics/draw.hpp"
#include "graphics/shader.hpp"

namespace nfwk {

void sprite_animation::update(float delta) {
	if (paused || done) {
		return;
	}
	previous_frame = current_frame;
	sub_frame += fps * delta;
	current_frame = static_cast<int>(sub_frame);
	if (!looping && current_frame >= frames) {
		current_frame--;
		done = true;
		return;
	}
	if (current_frame >= frames) {
		current_frame = 0;
		sub_frame = 0.0f;
	}
	if (previous_frame != current_frame) {
		set_tex_coords(uv_position, uv_size);
	}
}

void sprite_animation::draw(shader& shader, vector2f position, vector2f size) const {
	shader.draw(rectangle, transform2{ position, size });
}

void sprite_animation::draw(shader& shader, vector2f position, std::shared_ptr<texture> texture) const {
	shader.draw(rectangle, transform2{ position, texture->size().to<float>() });
}

void sprite_animation::draw(shader& shader, const transform2& transform) const {
	shader.draw(rectangle, transform);
}

void sprite_animation::pause() {
	paused = true;
}

void sprite_animation::resume() {
	paused = false;
}

bool sprite_animation::is_paused() const {
	return paused;
}

void sprite_animation::set_frame(int frame) {
	if (frame >= 0 && frame < frames) {
		previous_frame = current_frame;
		current_frame = frame;
		sub_frame = static_cast<float>(frame);
		set_tex_coords(uv_position, uv_size);
	}
}

void sprite_animation::set_tex_coords(vector2f position, vector2f size) {
	uv_position = position;
	uv_size = size;
	const float frame_width{ uv_size.x / static_cast<float>(frames) };
	rectangle.set_tex_coords(uv_position.x + frame_width * static_cast<float>(current_frame), uv_position.y, frame_width, uv_size.y);
}

void sprite_animation::set_tex_coords(vector4f tex_coords) {
	set_tex_coords(tex_coords.xy, tex_coords.zw);
}

void sprite_animation::start_looping() {
	looping = true;
	done = false;
}

void sprite_animation::stop_looping() {
	looping = false;
	done = false;
}

bool sprite_animation::is_looping() const {
	return looping;
}

bool sprite_animation::is_done() const {
	return done;
}

}
