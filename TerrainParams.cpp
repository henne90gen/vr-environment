#include "TerrainParams.h"

#include <array>
#include <random>

TerrainParams::TerrainParams() {
	noiseLayers = {
		  NoiseLayer(450.0F, 20.0F), //
		  NoiseLayer(300.0F, 15.0F), //
		  NoiseLayer(200.0F, 10.0F), //
		  NoiseLayer(150.0F, 7.5F),  //
		  NoiseLayer(100.0F, 5.0F),  //
		  NoiseLayer(80.0F, 4.0F),   //
		  NoiseLayer(30.0F, 2.0F),   //
		  NoiseLayer(7.5F, 0.75F),   //
	};
}

void TerrainParams::showGui() {
	// FIXME imgui
#if 0
    ImGui::DragFloat("Power", &power, 0.001F);
    ImGui::DragFloat("Bowl Strength", &bowlStrength, 0.1F);
    ImGui::SliderFloat("Platform Height", &platformHeight, 0.0F, 1.0F);
    ImGui::DragInt("Seed", &seed);
    if (ImGui::Button("New Seed")) {
        std::random_device rand_dev;
        std::mt19937 generator(rand_dev());
        std::uniform_int_distribution<int> distr(0, 1000000);
        seed = distr(generator);
    }
#endif
}

void TerrainParams::showLayersGui() {
// FIXME imgui
#if 0
	ImGui::Begin("Layers");
	int layerToRemove = -1;
	for (int i = 0; i < static_cast<int>(noiseLayers.size()); i++) {
		ImGui::Separator();

		const auto btnLabel = "Remove Layer " + std::to_string(i);
		if (ImGui::Button(btnLabel.c_str())) {
			layerToRemove = i;
		}

		ImGui::SameLine();
		const auto enabledLabel = "Enabled " + std::to_string(i);
		ImGui::Checkbox(enabledLabel.c_str(), &noiseLayers[i].enabled);

		const auto frequencyLabel = "Frequency " + std::to_string(i);
		ImGui::SliderFloat(frequencyLabel.c_str(), &noiseLayers[i].frequency, 0.0F, 500.0F);

		const auto amplitudeLabel = "Amplitude " + std::to_string(i);
		ImGui::SliderFloat(amplitudeLabel.c_str(), &noiseLayers[i].amplitude, 0.0F, 100.0F);
	}
	if (layerToRemove >= 0) {
		noiseLayers.erase(noiseLayers.begin() + layerToRemove);
	}

	ImGui::Separator();
	static int layerType = 0;
	static const std::array<const char *, 2> items = {"Noise", "Ridge"};
	ImGui::Combo("", &layerType, items.data(), items.size());
	ImGui::SameLine();
	if (ImGui::Button("Add Layer")) {
		switch (layerType) {
		case 0:
			noiseLayers.emplace_back();
			break;
		default:
			std::cout << "Invalid layer type" << std::endl;
			break;
		}
	}
	ImGui::End();
#endif
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
