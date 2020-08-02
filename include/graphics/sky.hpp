#pragma once

namespace no {

class perspective_camera;

void create_skybox();
void delete_skybox();
void draw_skybox(const perspective_camera& camera, float size, int shader, int texture);

}
