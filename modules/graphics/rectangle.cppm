export module nfwk.graphics:rectangle;

import :vertex;

export namespace nfwk {

class rectangle {
public:

	rectangle(float x, float y, float width, float height) {
		set_tex_coords(x, y, width, height);
	}
	rectangle() {
		set_tex_coords(0.0f, 0.0f, 1.0f, 1.0f);
	}

	void set_tex_coords(float x, float y, float width, float height) {
		/*vertices.set({
			{ { 0.0f, 0.0f }, 1.0f, { x, y } },
			{ { 1.0f, 0.0f }, 1.0f, { x + width, y } },
			{ { 1.0f, 1.0f }, 1.0f, { x + width, y + height } },
			{ { 0.0f, 1.0f }, 1.0f, { x, y + height } }
		}, { 0, 1, 2, 3, 2, 0 });*/
	}

	void bind() const {
		//vertices.bind();
	}

	void draw() const {
		//vertices.draw();
	}


private:

	//vertex_array<sprite_vertex, unsigned short> vertices;

};

}
