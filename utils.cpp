#include "utils.h"

cgv::render::render_types::vec3 to_vec3(glm::vec3 v) { return cgv::render::render_types::vec3(v.x, v.y, v.z); }
cgv::render::render_types::vec2 to_vec2(glm::vec2 v) { return cgv::render::render_types::vec2(v.x, v.y); }
cgv::render::render_types::ivec3 to_ivec3(glm::ivec3 v) { return cgv::render::render_types::ivec3(v.x, v.y, v.z); }

PerformanceCounter performanceCounter = {};
const PerformanceCounter &get_performance_counter() { return performanceCounter; }

Timer::Timer(std::string name) : name(std::move(name)) { start = std::chrono::high_resolution_clock::now(); }

Timer::~Timer() {
	auto end = std::chrono::high_resolution_clock::now();
	auto endNs = NANOSECONDS(end);
	auto startNs = NANOSECONDS(start);
	performanceCounter.recordValue(name, startNs, endNs);
#if 0
	int64_t duration = endNs - startNs;
	const double conversionFactor = 0.000001;
	double ms = static_cast<double>(duration) * conversionFactor;
	std::cout << name.c_str() << ": " << ms << "ms\n";
#endif
}

void PerformanceCounter::recordValue(const std::string &name, long long start, long long end) {
	if (dataPoints.find(name) == dataPoints.end()) {
		dataPoints[name] = {};
	}
	double valueMicro = static_cast<double>(end - start) * 0.001;
	double value = valueMicro * 0.001;

	auto &dp = dataPoints[name];
	dp.timerCount++;

	dp.lastValue = value;

	dp._sum += value;
	dp.average = dp._sum / dp.timerCount;

	dp._sdSum += (value - dp.average) * (value - dp.average);
	dp.standardDeviation = sqrt(dp._sdSum / dp.timerCount);
}

void PerformanceCounter::reset() {
	for (auto &dataPoint : dataPoints) {
		dataPoint.second = {};
	}
}

void PerformanceCounter::print() const {
	for (auto &dataPoint : dataPoints) {
		std::cerr << dataPoint.first << ": avg=" << dataPoint.second.average
				  << "ms | sd=" << dataPoint.second.standardDeviation << "ms" << std::endl;
	}
}
