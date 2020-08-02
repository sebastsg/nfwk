#pragma once

#include "graphics/shader_variable.hpp"

#include <filesystem>

namespace no {

class ortho_camera;
class perspective_camera;

enum class polygon_render_mode { fill, wireframe };

int create_shader_from_source(std::string_view vertex, std::string_view fragment);
int create_shader(const std::filesystem::path& path);
void bind_shader(int id);
void delete_shader(int id);
shader_variable get_shader_variable(const std::string& name);
void set_shader_model(const glm::mat4& transform);
void set_shader_model(const transform2& transform);
void set_shader_model(const transform3& transform);
void set_shader_view_projection(const glm::mat4& view, const glm::mat4& projection);
void set_shader_view_projection(const ortho_camera& camera);
void set_shader_view_projection(const perspective_camera& camera);

void set_polygon_render_mode(polygon_render_mode mode);

}
