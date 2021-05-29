#include "utils.h"

cgv::render::render_types::vec3 to_vec3(glm::vec3 v) { return cgv::render::render_types::vec3(v.x, v.y, v.z); }
cgv::render::render_types::vec2 to_vec2(glm::vec2 v) { return cgv::render::render_types::vec2(v.x, v.y); }
cgv::render::render_types::ivec3 to_ivec3(glm::ivec3 v) { return cgv::render::render_types::ivec3(v.x, v.y, v.z); }

Timer::Timer(std::string name) : name(std::move(name)) { start = std::chrono::high_resolution_clock::now(); }

Timer::~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto endNs = NANOSECONDS(end);
    auto startNs = NANOSECONDS(start);
    int64_t duration = endNs - startNs;
    const double conversionFactor = 0.000001;
    double ms = static_cast<double>(duration) * conversionFactor;
    std::cerr << name.c_str() << ": " << ms << "ms\n";
}
