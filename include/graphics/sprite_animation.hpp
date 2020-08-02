#pragma once

#include "transform.hpp"
#include "graphics/rectangle.hpp"

namespace no {

class sprite_animation {
public:

	int frames{ 1 };
	float fps{ 10.0f };

	void update(float delta);
	void draw(vector2f position, vector2f size) const;
	void draw(vector2f position, int texture) const;
	void draw(const transform2& transform) const;

	void pause();
	void resume();
	bool is_paused() const;
	void set_frame(int frame);
	void set_tex_coords(vector2f position, vector2f size);
	void set_tex_coords(vector4f tex_coords);
	void start_looping();
	void stop_looping();
	bool is_looping() const;
	bool is_done() const;

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
