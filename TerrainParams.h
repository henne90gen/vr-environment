#pragma once

#include <memory>
#include <vector>

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>

#include <gl/Shader.h>

#include "Layers.h"

struct TerrainParams {
	std::vector<NoiseLayer> noiseLayers;
	float power = 1.1F;
	float bowlStrength = 20.0F;
	float platformHeight = 0.15F;
	int seed = 1337;

	TerrainParams();

	void set_shader_uniforms(cgv::render::context &ctx, cgv::render::shader_program &program) const;
};
