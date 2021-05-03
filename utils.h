#pragma once

#include <cgv/render/render_types.h>
#include <glm/glm.hpp>

cgv::render::render_types::vec3 to_vec3(glm::vec3 v) { return cgv::render::render_types::vec3(v.x, v.y, v.z); }
cgv::render::render_types::vec2 to_vec2(glm::vec2 v) { return cgv::render::render_types::vec2(v.x, v.y); }
cgv::render::render_types::ivec3 to_ivec3(glm::ivec3 v) { return cgv::render::render_types::ivec3(v.x, v.y, v.z); }
