export module nfwk.graphics:sprite_animation;

import std.core;
import nfwk.core;
import :rectangle;

export namespace nfwk {

// todo: split into two classes

class sprite_animation {
public:

	int frames{ 1 };
	float fps{ 10.0f };

	void update(float delta) {
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

	void draw(vector2f position, vector2f size) const {
		//draw_shape(rectangle, transform2{ position, size });
	}

	void draw(vector2f position, int texture) const {
		//draw_shape(rectangle, transform2{ position, texture_size(texture).to<float>() });
	}

	void draw(const transform2& transform) const {
		//draw_shape(rectangle, transform);
	}

	void pause() {
		paused = true;
	}

	void resume() {
		paused = false;
	}

	bool is_paused() const {
		return paused;
	}

	void set_frame(int frame) {
		if (frame >= 0 && frame < frames) {
			previous_frame = current_frame;
			current_frame = frame;
			sub_frame = static_cast<float>(frame);
			set_tex_coords(uv_position, uv_size);
		}
	}

	void set_tex_coords(vector2f position, vector2f size) {
		uv_position = position;
		uv_size = size;
		const float frame_width{ uv_size.x / static_cast<float>(frames) };
		rectangle.set_tex_coords(uv_position.x + frame_width * static_cast<float>(current_frame), uv_position.y, frame_width, uv_size.y);
	}

	void set_tex_coords(vector4f tex_coords) {
		set_tex_coords(tex_coords.xy, tex_coords.zw);
	}

	void start_looping() {
		looping = true;
		done = false;
	}

	void stop_looping() {
		looping = false;
		done = false;
	}

	bool is_looping() const {
		return looping;
	}

	bool is_done() const {
		return done;
	}

	vector4f get_tex_coords() const {
		const float frame_width{ uv_size.x / static_cast<float>(frames) };
		return { uv_position.x + frame_width * static_cast<float>(current_frame), uv_position.y, frame_width, uv_size.y };
	}

private:

	rectangle rectangle;
	int current_frame{ 0 };
	float sub_frame{ 0.0f };
	int previous_frame{ 0 };
	bool paused{ false };
	vector2f uv_position;
	vector2f uv_size{ 1.0f };
	bool looping{ true };
	bool done{ false };

};

}
