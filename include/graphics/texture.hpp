#pragma once

#include "vector2.hpp"

namespace no {

class surface;

enum class scale_option { nearest_neighbour, linear };

int create_texture();
int create_texture(const surface& surface, scale_option scaling, bool mipmap);
int create_texture(const surface& surface);
void bind_texture(int id);
void bind_texture(int id, int slot);
void load_texture(int id, const surface& surface, scale_option scaling, bool mipmap);
void load_texture(int id, const surface& surface);
void load_texture_from_screen(int id, int bottom_y, int x, int y, int width, int height);
vector2i texture_size(int id);
void delete_texture(int id);

}
