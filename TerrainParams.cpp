#include "TerrainParams.h"

#include <array>
#include <random>

TerrainParams::TerrainParams() {
	noiseLayers = {
		  NoiseLayer(1.0F / 450.0F, 20.0F), //
		  NoiseLayer(1.0F / 300.0F, 15.0F), //
		  NoiseLayer(1.0F / 200.0F, 10.0F), //
		  NoiseLayer(1.0F / 150.0F, 7.5F),  //
		  NoiseLayer(1.0F / 100.0F, 5.0F),  //
		  NoiseLayer(1.0F / 80.0F, 4.0F),   //
		  NoiseLayer(1.0F / 30.0F, 2.0F),   //
		  NoiseLayer(1.0F / 7.5F, 0.75F),   //
	};
}

void TerrainParams::set_shader_uniforms(cgv::render::context &ctx, cgv::render::shader_program &program) const {
	for (int i = 0; i < static_cast<int64_t>(noiseLayers.size()); i++) {
		program.set_uniform(ctx, "noiseLayers[" + std::to_string(i) + "].amplitude", noiseLayers[i].amplitude);
		program.set_uniform(ctx, "noiseLayers[" + std::to_string(i) + "].frequency", noiseLayers[i].frequency);
		program.set_uniform(ctx, "noiseLayers[" + std::to_string(i) + "].enabled", noiseLayers[i].enabled);
	}
	program.set_uniform(ctx, "numNoiseLayers", static_cast<int>(noiseLayers.size()));
	program.set_uniform(ctx, "power", power);
	program.set_uniform(ctx, "bowlStrength", bowlStrength);
	program.set_uniform(ctx, "platformHeight", platformHeight);
	program.set_uniform(ctx, "seed", seed);
}
