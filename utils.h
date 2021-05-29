#pragma once

#include <cgv/render/render_types.h>
#include <chrono>
#include <glm/glm.hpp>

cgv::render::render_types::vec3 to_vec3(glm::vec3 v);
cgv::render::render_types::vec2 to_vec2(glm::vec2 v);
cgv::render::render_types::ivec3 to_ivec3(glm::ivec3 v);

#define NANOSECONDS(timePoint)                                                                                         \
	std::chrono::time_point_cast<std::chrono::nanoseconds>(timePoint).time_since_epoch().count()

struct DataPoint {
	double lastValue = 0.0;
	double average = 0.0;
	double standardDeviation = 0.0;
	double _sum = 0.0;
	double _sdSum = 0.0;
	unsigned int timerCount = 0;
};

class PerformanceCounter {
  public:
	PerformanceCounter() = default;
	~PerformanceCounter() = default;

	void recordValue(const std::string &name, long long start, long long end);
	void reset();
	void print() const;

	std::unordered_map<std::string, DataPoint> dataPoints = {};
};

const PerformanceCounter &get_performance_counter();

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
