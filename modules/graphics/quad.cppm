export module nfwk.graphics:quad;

import :vertex;

export namespace nfwk {

template<typename Vertex>
class quad {
public:

	quad() {
		set({}, {}, {}, {});
	}

	quad(const Vertex& top_left, const Vertex& top_right, const Vertex& bottom_left, const Vertex& bottom_right) {
		set(top_left, top_right, bottom_right, bottom_left);
	}

	void set(const Vertex& top_left, const Vertex& top_right, const Vertex& bottom_left, const Vertex& bottom_right) {
		//vertices.set({ top_left, top_right, bottom_right, bottom_left }, { 0, 1, 2, 3, 2, 0 });
	}

	void bind() const {
		//vertices.bind();
	}

	void draw() const {
		//vertices.draw();
	}

private:

	//vertex_array<Vertex, unsigned short> vertices;

};

}
