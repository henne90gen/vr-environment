#pragma once

struct NoiseLayer {
	bool enabled = true;
	float frequency = 1.0F;
	float amplitude = 1.0F;

	NoiseLayer() = default;
	NoiseLayer(float frequency, float amplitude) : frequency(frequency), amplitude(amplitude) {}
};
