export module nfwk.scripts:object_renderer;

import std.core;
import nfwk.core;
import nfwk.graphics;
import nfwk.draw;
import :object_instance;

export namespace nfwk::objects {

class object_class;

class rendered_object {
public:

	transform2 transform;

	rendered_object(const transform2& transform) : transform{ transform } {}

};

class object_renderer {
public:

	void render() {
		/*objects.clear();
		for (const auto& instance : get_instances()) {
			objects.emplace_back(instance.transform);
		}*/
	}

	void draw() {
		/*for (const auto& object : objects) {
			draw_shape(shape, object.transform);
		}*/
	}

private:

	std::vector<rendered_object> objects;
	rectangle shape;

};

}
