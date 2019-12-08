#pragma once

#include "math.hpp"

#include <functional>
#include <vector>

namespace no {

template<typename Object>
class object_list {
public:

	void with(int id, const std::function<void(Object&)>& function) {
		if (auto instance{ find(id) }) {
			function(instance->object);
		}
	}

	void for_each(const std::function<void(Object&, int)>& function) {
		for (auto& instance : instances) {
			function(instance->object, instance->unique_id);
		}
	}

	void for_each_until(const std::function<bool(Object&, int)>& function) {
		for (auto& instance : instances) {
			if (!function(instance->object, instance->unique_id)) {
				break;
			}
		}
	}

	void delete_if(const std::function<bool(Object&, int)>& function) {
		for (auto it{ instances.begin() }; it != instances.end();) {
			if (function(it->object, it->unique_id)) {
				it = instances.erase(it);
			} else {
				++it;
			}
		}
	}

private:

	struct object_instance {
		Object object;
		int unique_id{ -1 };
	};

	std::vector<object_instance> instances;

	object_instance* find(int id) {
		return id >= 0 && id < static_cast<int>(instances.size()) ? &instances[id] : nullptr;
	}

};

}
