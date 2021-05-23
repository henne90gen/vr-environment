#include "Trees.h"

#include <array>
#include <random>

#include <cgv_gl/box_wire_renderer.h>

#include "Sphere.h"
#include "macros.h"
#include "renderers/tree_renderer.h"
#include "utils.h"

bool Trees::init(cgv::render::context &ctx) {
	auto &box_wire = ref_box_wire_renderer(ctx, 1);
	if (!box_wire.init(ctx)) {
		std::cerr << "failed to init box_wire_renderer" << std::endl;
		return false;
	}

	auto &tree_renderer = ref_tree_renderer(ctx, 1);
	if (!tree_renderer.init(ctx)) {
		std::cerr << "failed to init tree_renderer" << std::endl;
		return false;
	}

	initGrid();
	initTreeMesh();
	initCubeMesh();

	if (!initComputeShader(ctx)) {
		std::cerr << "failed to init tree placement compute shader" << std::endl;
		return false;
	}

	return true;
}

void Trees::render(cgv::render::context &ctx, const ShaderToggles &shaderToggles, const TerrainParams &terrainParams) {
	if (!enabled) {
		return;
	}

	// TODO(henne): compute shader execution can be moved into init
	renderComputeShader(ctx, terrainParams);

	renderGrid(ctx);

	auto &tr = ref_tree_renderer(ctx);
	tr.set_position_texture(ctx, tree_position_texture);
	auto &mesh = tree_mesh;
	if (showCubes) {
		mesh = cube_mesh;
	}

	tr.set_position_array(ctx, mesh.positions);
	tr.set_normal_array(ctx, mesh.normals);
	tr.set_texcoord_array(ctx, mesh.uvs);
	tr.set_indices(ctx, mesh.indices);
	tr.render(ctx, 0, mesh.indices.size());
}

bool Trees::initComputeShader(cgv::render::context &ctx) {
	if (!tree_position_compute_shader.is_created()) {
		if (!tree_position_compute_shader.build_program(ctx, "tree_placement.glpr", true)) {
			std::cerr << "could not build program tree_placement.glpr" << std::endl;
			return false;
		}
	}

	tree_position_texture = cgv::render::texture( //
		  "flt32[R,G,B,A]",                       //
		  cgv::render::TF_NEAREST,                //
		  cgv::render::TF_NEAREST,                //
		  cgv::render::TW_CLAMP_TO_EDGE,          //
		  cgv::render::TW_CLAMP_TO_EDGE           //
	);
	if (!tree_position_texture.create(ctx, cgv::render::TT_2D, treePositionTextureWidth, treePositionTextureHeight)) {
		std::cerr << "failed to create tree position texture" << std::endl;
		return false;
	}
	glBindImageTexture(0, get_gl_id(tree_position_texture.handle), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	return true;
}

void Trees::renderComputeShader(cgv::render::context &ctx, const TerrainParams &terrainParams) {
	if (!tree_position_compute_shader.enable(ctx)) {
		std::cout << "failed to enable tree position compute shader" << std::endl;
		return;
	}

	tree_position_compute_shader.set_uniform(ctx, "treeCount", treeCount);
	tree_position_compute_shader.set_uniform(ctx, "lodSize", lodSize);
	tree_position_compute_shader.set_uniform(ctx, "lodInnerSize", lodInnerSize);
	terrainParams.set_shader_uniforms(ctx, tree_position_compute_shader);

	glBindImageTexture(0, get_gl_id(tree_position_texture.handle), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glDispatchCompute(4, 4, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	if (!tree_position_compute_shader.disable(ctx)) {
		std::cout << "failed to disable tree position compute shader" << std::endl;
		return;
	}
}

void Trees::initGrid() {
	const float lodH = lodSize / 2.0F;
	const float lodIH = lodInnerSize / 2.0F;
	const float smallSideLength = (lodSize - lodInnerSize) / 2.0F;
	std::vector<glm::vec4> gridOffsets = {
		  {-lodIH, -lodIH, lodInnerSize, lodInnerSize},

		  {-lodH, lodIH, smallSideLength, smallSideLength}, {-lodIH, lodIH, lodInnerSize, smallSideLength},
		  {lodIH, lodIH, smallSideLength, smallSideLength},

		  {-lodH, -lodIH, smallSideLength, lodInnerSize},   {lodIH, -lodIH, smallSideLength, lodInnerSize},

		  {-lodH, -lodH, smallSideLength, smallSideLength}, {-lodIH, -lodH, lodInnerSize, smallSideLength},
		  {lodIH, -lodH, smallSideLength, smallSideLength},
	};
	for (const auto &gridOffset : gridOffsets) {
		gridBoxes.emplace_back(                                                          //
			  vec3(gridOffset.x, gridHeight, gridOffset.y),                              //
			  vec3(gridOffset.x + gridOffset.z, gridHeight, gridOffset.y + gridOffset.w) //
		);
	}
}

void Trees::renderGrid(cgv::render::context &ctx) {
	if (!showGrid) {
		return;
	}

	auto &box_wire = cgv::render::ref_box_wire_renderer(ctx);
	box_wire.set_box_array(ctx, gridBoxes);
	box_wire.render(ctx, 0, gridBoxes.size());
}

void appendSphereLeaf(const glm::mat4 modelMatrix, Trees::Mesh &tm, int positionOffset, int indexOffset) {
	std::vector<glm::vec3> vertices = {};
	std::vector<glm::vec3> sphereNormals = {};
	std::vector<glm::vec2> sphereUvs = {};
	std::vector<glm::ivec3> sphereIndices = {};

	Sphere s = {5, 3};
	s.append(vertices, sphereNormals, sphereUvs, sphereIndices);

	for (int i = 0; i < vertices.size(); i++) {
		const auto &vertex = vertices[i];
		tm.positions[positionOffset + i] = to_vec3(glm::vec3(modelMatrix * glm::vec4(vertex, 1.0)));

		const auto &normal = sphereNormals[i];
		tm.normals[positionOffset + i] = to_vec3(glm::vec3(modelMatrix * glm::vec4(normal, 0.0F)));

		// TODO randomly rotate uv coordinates to make the leaves less uniform
		auto uv = sphereUvs[i];
		uv.x *= 0.3289;
		uv.y *= 0.4358;
		uv += glm::vec2(0.6711, 0.5);
		tm.uvs[positionOffset + i] = to_vec2(uv);
	}

#pragma omp parallel for
	for (int i = 0; i < sphereIndices.size(); i++) {
		auto sphereIndex = sphereIndices[i] + positionOffset;
		tm.indices[indexOffset + i * 3 + 0] = sphereIndex.x;
		tm.indices[indexOffset + i * 3 + 1] = sphereIndex.y;
		tm.indices[indexOffset + i * 3 + 2] = sphereIndex.z;
	}
}

void Trees::initTreeMesh() {
	std::vector<glm::mat4> leafModelMatrices = {};
	{
		std::vector<glm::vec3> positions = {};
		std::vector<glm::vec3> normals = {};
		std::vector<glm::vec2> uvs = {};
		std::vector<glm::ivec3> indices = {};
		Tree *tree = Tree::create(treeSettings);
		tree->construct(positions, normals, uvs, indices);
		tree->addLeaves(leafModelMatrices);

		// adjust uv coordinates to be constrained to the bark side of the texture
#pragma omp parallel for
		for (int i = 0; i < uvs.size(); i++) {
			uvs[i].x *= 0.671;
		}

		tree_mesh.assign(positions, normals, uvs, indices);
	}

	int leafPositionOffset = static_cast<int>(tree_mesh.positions.size());
	int leafIndicesOffset = static_cast<int>(tree_mesh.indices.size());

	int sectorCount = 7;
	int stackCount = 5;
	int verticesPerLeaf = (sectorCount + 1) * (stackCount + 1);
	int trianglesPerLeaf = (stackCount - 1) * sectorCount * 2;
	int totalVertices = leafPositionOffset + static_cast<int>(leafModelMatrices.size()) * verticesPerLeaf;
	int totalIndices = leafIndicesOffset + static_cast<int>(leafModelMatrices.size()) * (trianglesPerLeaf * 3);

	tree_mesh.positions.resize(totalVertices);
	tree_mesh.normals.resize(totalVertices);
	tree_mesh.uvs.resize(totalVertices);
	tree_mesh.indices.resize(totalIndices);

#pragma omp parallel for
	for (int i = 0; i < leafModelMatrices.size(); i++) {
		const auto &modelMatrix = leafModelMatrices[i];
		int positionOffset = leafPositionOffset + i * verticesPerLeaf;
		int indexOffset = leafIndicesOffset + i * trianglesPerLeaf * 3;
		appendSphereLeaf(modelMatrix, tree_mesh, positionOffset, indexOffset);
	}
}

void Trees::initCubeMesh() {
	static std::vector<glm::vec3> p = {
		  // back
		  {-0.5F, -0.5F, -0.5F}, // 0
		  {-0.5F, 0.5F, -0.5F},  // 1
		  {0.5F, 0.5F, -0.5F},   // 2
		  {0.5F, -0.5F, -0.5F},  // 3

		  // front
		  {-0.5F, -0.5F, 0.5F}, // 4
		  {0.5F, -0.5F, 0.5F},  // 5
		  {0.5F, 0.5F, 0.5F},   // 6
		  {-0.5F, 0.5F, 0.5F},  // 7

		  // left
		  {-0.5F, -0.5F, -0.5F}, // 8
		  {-0.5F, -0.5F, 0.5F},  // 9
		  {-0.5F, 0.5F, 0.5F},   // 10
		  {-0.5F, 0.5F, -0.5F},  // 11

		  // right
		  {0.5F, -0.5F, 0.5F},  // 12
		  {0.5F, -0.5F, -0.5F}, // 13
		  {0.5F, 0.5F, -0.5F},  // 14
		  {0.5F, 0.5F, 0.5F},   // 15

		  // top
		  {-0.5F, 0.5F, 0.5F},  // 16
		  {0.5F, 0.5F, 0.5F},   // 17
		  {0.5F, 0.5F, -0.5F},  // 18
		  {-0.5F, 0.5F, -0.5F}, // 19

		  // bottom
		  {-0.5F, -0.5F, 0.5F},  // 20
		  {-0.5F, -0.5F, -0.5F}, // 21
		  {0.5F, -0.5F, -0.5F},  // 22
		  {0.5F, -0.5F, 0.5F},   // 23
	};
	static std::vector<glm::vec3> n = {
		  {0, 0, -1.0}, {0, 0, -1.0}, {0, 0, -1.0}, {0, 0, -1.0}, // back
		  {0, 0, 1.0},  {0, 0, 1.0},  {0, 0, 1.0},  {0, 0, 1.0},  // front
		  {-1.0, 0, 0}, {-1.0, 0, 0}, {-1.0, 0, 0}, {-1.0, 0, 0}, // left
		  {1.0, 0, 0},  {1.0, 0, 0},  {1.0, 0, 0},  {1.0, 0, 0},  // right
		  {0, 1.0, 0},  {0, 1.0, 0},  {0, 1.0, 0},  {0, 1.0, 0},  // top
		  {0, -1.0, 0}, {0, -1.0, 0}, {0, -1.0, 0}, {0, -1.0, 0}, // bottom
	};
	std::vector<glm::vec2> uvs = {};
	for (int i = 0; i < 6; i++) {
		uvs.emplace_back(0.0, 1.0);
		uvs.emplace_back(1.0, 1.0);
		uvs.emplace_back(1.0, 0.0);
		uvs.emplace_back(0.0, 0.0);
	}
	static std::vector<glm::ivec3> indices = {
		  // front
		  {0, 1, 2},
		  {0, 2, 3},

		  // back
		  {4, 5, 6},
		  {4, 6, 7},

		  // left
		  {8, 9, 10},
		  {8, 10, 11},

		  // right
		  {12, 13, 14},
		  {12, 14, 15},

		  // top
		  {16, 17, 18},
		  {16, 18, 19},

		  // bottom
		  {20, 21, 22},
		  {20, 22, 23},
	};

	cube_mesh.clear();
	cube_mesh.assign(p, n, uvs, indices);
}

void Trees::clear(cgv::render::context &ctx) {
	ref_box_wire_renderer(ctx, -1);
	ref_tree_renderer(ctx, -1);
}

#include <cgv/gui/provider.h>

struct trees_gui_creator : public cgv::gui::gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(cgv::gui::provider *p, const std::string &label, void *value_ptr, const std::string &value_type,
				const std::string &gui_type, const std::string &options, bool *) override {
		if (value_type != cgv::type::info::type_name<Trees>::get_name())
			return false;
		auto *trees_ptr = reinterpret_cast<Trees *>(value_ptr);
		auto *b = dynamic_cast<cgv::base::base *>(p);
		p->add_member_control(b, "Enabled", trees_ptr->enabled);
		p->add_member_control(b, "Show Placement Grid", trees_ptr->showGrid);
		p->add_member_control(b, "Show Cubes instead of Trees", trees_ptr->showCubes);
		return true;
	}
};

cgv::gui::gui_creator_registration<trees_gui_creator> trees_gc_reg("trees_gui_creator");
