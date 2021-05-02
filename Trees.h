#pragma once

#include <cgv/render/context.h>
#include <cgv/render/render_types.h>
#include <cgv/render/shader_program.h>

#include <gl/Shader.h>
#include <gl/VertexArray.h>

#include <memory>

#include "Branch.h"
#include "landscape/ShaderToggles.h"
#include "landscape/TerrainParams.h"

class Trees : public cgv::render::render_types {
  public:
	bool enabled = true;
	bool showCubes = false;
	bool showGrid = true;

  private:
	cgv::render::shader_program tree_placement_compute_shader;
	cgv::render::texture tree_position_texture;
	unsigned int treePositionTextureWidth = 32;
	unsigned int treePositionTextureHeight = 32;

	std::shared_ptr<VertexArray> generatedTreesVA = nullptr;
	std::shared_ptr<Texture> barkTexture = nullptr;

	int treeCount = 1024;
	float lodSize = 1000.0F;
	float lodInnerSize = 100.0F;
	float treeScale = 0.15F;
	TreeSettings treeSettings = {};

	std::vector<box3> gridBoxes = {};
	float gridHeight = 120.0F;

  public:
	[[nodiscard]] const cgv::render::texture &get_tree_placement_texture() const { return tree_position_texture; }

	bool init(cgv::render::context &ctx);
	void clear(cgv::render::context &ctx);

	void render(cgv::render::context &ctx, const ShaderToggles &shaderToggles, const TerrainParams &terrainParams);

  private:
	bool initComputeShader(cgv::render::context &ctx);
	void initGrid();

	void renderComputeShader(cgv::render::context &ctx, const TerrainParams &terrainParams);
	void renderCubes(cgv::render::context &ctx, const ShaderToggles &shaderToggles);
	void renderGrid(cgv::render::context &ctx);
	void renderTrees(cgv::render::context &ctx, const ShaderToggles &shaderToggles);

	void generateTree();
};
