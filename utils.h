#pragma once

#include <cgv/render/render_types.h>
#include <chrono>
#include <glm/glm.hpp>

cgv::render::render_types::vec3 to_vec3(glm::vec3 v);
cgv::render::render_types::vec2 to_vec2(glm::vec2 v);
cgv::render::render_types::ivec3 to_ivec3(glm::ivec3 v);

#define NANOSECONDS(timePoint)                                                                                         \
	std::chrono::time_point_cast<std::chrono::nanoseconds>(timePoint).time_since_epoch().count()

class Timer {
  public:
	explicit Timer(std::string name);
	~Timer();

  private:
	std::string name;
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

#define COMBINE(X, Y) X##Y
#define TIME_SCOPE(name) auto COMBINE(timer, __LINE__) = Timer(name)
