#pragma once

#include <cgv/render/context.h>
#include <cgv/render/render_types.h>
#include <cgv/render/shader_program.h>

#include <memory>

#include "Branch.h"
#include "ShaderToggles.h"
#include "TerrainParams.h"

class Trees : public cgv::render::render_types {
  public:
	struct Mesh {
		std::vector<vec3> positions = {};
		std::vector<vec3> normals = {};
		std::vector<vec2> uvs = {};
		std::vector<unsigned int> indices = {};

		void clear() {
			positions.clear();
			normals.clear();
			uvs.clear();
			indices.clear();
		}

		void assign(std::vector<glm::vec3> &p, std::vector<glm::vec3> &n, std::vector<glm::vec2> &u,
					std::vector<glm::ivec3> &i) {
			positions.assign(reinterpret_cast<vec3 *>(p.data()), reinterpret_cast<vec3 *>(p.data() + p.size()));
			normals.assign(reinterpret_cast<vec3 *>(n.data()), reinterpret_cast<vec3 *>(n.data() + n.size()));
			uvs.assign(reinterpret_cast<vec2 *>(u.data()), reinterpret_cast<vec2 *>(u.data() + u.size()));
			indices.assign(reinterpret_cast<unsigned int *>(i.data()),
						   reinterpret_cast<unsigned int *>(i.data() + i.size()));
		}
	};

	bool enabled = true;
	bool showCubes = false;
	bool showGrid = true;

  private:
	std::vector<vec3> positions = {
		  {0.0, 0.0, 0.0},
		  {1.0, 0.0, 0.0},
		  {1.0, 1.0, 0.0},
		  {0.0, 1.0, 0.0},
	};
	std::vector<vec2> texcoords = {
		  {0.0, 1.0},
		  {1.0, 1.0},
		  {1.0, 0.0},
		  {0.0, 0.0},
	};
	std::vector<unsigned int> indices = {0, 1, 2, 0, 2, 3};

	cgv::render::shader_program tree_position_compute_shader;
	cgv::render::texture tree_position_texture;
	unsigned int treePositionTextureWidth = 32;
	unsigned int treePositionTextureHeight = 32;

    Mesh tree_mesh = {};
    Mesh cube_mesh = {};

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
    void initTreeMesh();
	void initCubeMesh();

	void renderComputeShader(cgv::render::context &ctx, const TerrainParams &terrainParams);
	void renderGrid(cgv::render::context &ctx);
};
