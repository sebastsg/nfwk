#pragma once

namespace nfwk {

class perspective_camera;
class shader;
class texture;

void create_skybox();
void delete_skybox();
void draw_skybox(const perspective_camera& camera, float size, shader& shader, const texture& texture);

}
