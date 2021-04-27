#pragma once

#include <gl/Shader.h>
#include <gl/VertexArray.h>

#include <memory>

#include "Branch.h"
#include "landscape/ShaderToggles.h"
#include "landscape/TerrainParams.h"

class Trees {
	std::shared_ptr<Shader> shader = nullptr;
	std::shared_ptr<Shader> flatColorShader = nullptr;

	std::shared_ptr<Shader> compShader = nullptr;
	unsigned int treePositionTextureWidth = 32;
	unsigned int treePositionTextureHeight = 32;
	unsigned int treePositionTextureId = 0;

	std::shared_ptr<VertexArray> cubeVA = nullptr;

	std::shared_ptr<VertexArray> generatedTreesVA = nullptr;
	std::shared_ptr<Texture> barkTexture = nullptr;

	int treeCount = 1024;
	float lodSize = 1000.0F;
	float lodInnerSize = 100.0F;
	float treeScale = 0.15F;
	TreeSettings treeSettings = {};

	std::shared_ptr<VertexArray> gridVA = nullptr;
	float gridHeight = 120.0F;

	// TODO add these two toggles to the UI
	bool showTrees = true;
	bool showGrid = false;
	bool usingGeneratedTrees = true;

  public:
	[[nodiscard]] unsigned int getTreePositionTextureId() const { return treePositionTextureId; }

	void init();
	void showGui();

	void render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
				const TerrainParams &terrainParams);

  private:
	void initComputeShader();
	void initModel();
	void initGrid();

	void renderComputeShader(const TerrainParams &terrainParams);
	void renderCubes(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
					 const ShaderToggles &shaderToggles);
	void renderGrid(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix);
	void renderTrees(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
					 const ShaderToggles &shaderToggles);

	void generateTrees();
};
