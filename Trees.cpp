#include "Trees.h"

#include <array>
#include <random>

#include <Image.h>
#include <ImageOps.h>
#include <util/RenderUtils.h>

#include <cgv_gl/box_wire_renderer.h>

#include "macros.h"
#include "renderers/tree_renderer.h"
#include "utils.h"

// FIXME shader
//  DEFINE_DEFAULT_SHADERS(landscape_Tree)
//  DEFINE_DEFAULT_SHADERS(landscape_Texture)
//  DEFINE_DEFAULT_SHADERS(landscape_FlatColor)
//  DEFINE_SHADER(landscape_NoiseLib)
//  DEFINE_SHADER(landscape_TreeComp)

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

	if (!initComputeShader(ctx)) {
		std::cerr << "failed to init tree placement compute shader" << std::endl;
		return false;
	}

	return true;

	// FIXME finish init code
#if 0
	barkTexture = std::make_shared<Texture>();

	Image img = {};
	if (ImageOps::load("./landscape_resources/assets/textures/bark_and_leafs_light.png", img)) {
		img.applyToTexture(barkTexture);
	}
#endif
}

void Trees::render(cgv::render::context &ctx, const ShaderToggles &shaderToggles, const TerrainParams &terrainParams) {
	if (!enabled) {
		return;
	}

	// TODO(henne): compute shader execution can be moved into init
	renderComputeShader(ctx, terrainParams);

	renderGrid(ctx);

	if (showCubes) {
		renderCubes(ctx, shaderToggles);
	} else {
		generateTree();

		auto &tr = ref_tree_renderer(ctx);
		tr.set_position_texture(ctx, tree_position_texture);
		tr.set_position_array(ctx, tree_mesh.positions);
		tr.set_normal_array(ctx, tree_mesh.normals);
		tr.set_texcoord_array(ctx, tree_mesh.uvs);
		tr.set_indices(ctx, tree_mesh.indices);
		tr.render(ctx, 0, tree_mesh.indices.size());
	}
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
	GL_Call(glBindImageTexture(0, get_gl_id(tree_position_texture.handle), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));

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

	GL_Call(glBindImageTexture(0, get_gl_id(tree_position_texture.handle), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F));
	GL_Call(glDispatchCompute(4, 4, 1));
	GL_Call(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));

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

void Trees::renderCubes(cgv::render::context &ctx, const ShaderToggles &shaderToggles) {
	// FIXME use box_renderer here

#if 0
	cubeVA->bind();
	shader->bind();
	cubeVA->setShader(shader);
	auto modelMatrix = glm::identity<glm::mat4>();
	shader->setUniform("modelMatrix", modelMatrix);
	shader->setUniform("viewMatrix", viewMatrix);
	shader->setUniform("projectionMatrix", projectionMatrix);
	shader->setUniform("treeCount", treeCount);
	shader->setUniform("positionTexture", 0);

	GL_Call(glActiveTexture(GL_TEXTURE0));
	GL_Call(glBindTexture(GL_TEXTURE_2D, treePositionTextureId));

	if (shaderToggles.drawWireframe) {
		GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	}

	GL_Call(glDrawElementsInstanced(GL_TRIANGLES, cubeVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr,
									treeCount));

	if (shaderToggles.drawWireframe) {
		GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	}
#endif
}

void appendLeaf(int positionOffset, int indexOffset, std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals,
				std::vector<glm::ivec3> &indices, const glm::mat4 &modelMatrix) {
	const std::array<glm::vec3, 11> leafVertices = {
		  glm::vec3(-0.1F, 0.0F, 0.0F), //
		  glm::vec3(0.1F, 0.0F, 0.0F),  //
		  glm::vec3(0.1F, 0.0F, 1.0F),  //
		  glm::vec3(-0.1F, 0.0F, 1.0F), //
		  glm::vec3(-0.4F, 0.0F, 0.5F), //
		  glm::vec3(-1.0F, 0.0F, 0.8F), //
		  glm::vec3(-1.0F, 0.0F, 1.7F), //
		  glm::vec3(0.0F, 0.0F, 2.5F),  //
		  glm::vec3(1.0F, 0.0F, 1.7F),  //
		  glm::vec3(1.0F, 0.0F, 0.8F),  //
		  glm::vec3(0.4F, 0.0F, 0.5F),  //
	};
	const glm::vec3 quadNormal = {0.0F, 1.0F, 0.0F};

	for (int i = 0; i < leafVertices.size(); i++) {
		positions[positionOffset + i] = glm::vec3(modelMatrix * glm::vec4(leafVertices[i], 1.0));
		normals[positionOffset + i] = glm::vec3(modelMatrix * glm::vec4(quadNormal, 0.0));
	}

	indices[indexOffset + 0] = glm::ivec3(positionOffset + 0, positionOffset + 1, positionOffset + 2);
	indices[indexOffset + 1] = glm::ivec3(positionOffset + 0, positionOffset + 2, positionOffset + 3);
	indices[indexOffset + 2] = glm::ivec3(positionOffset + 4, positionOffset + 3, positionOffset + 5);
	indices[indexOffset + 3] = glm::ivec3(positionOffset + 5, positionOffset + 3, positionOffset + 6);
	indices[indexOffset + 4] = glm::ivec3(positionOffset + 6, positionOffset + 3, positionOffset + 7);
	indices[indexOffset + 5] = glm::ivec3(positionOffset + 7, positionOffset + 3, positionOffset + 2);
	indices[indexOffset + 6] = glm::ivec3(positionOffset + 7, positionOffset + 2, positionOffset + 8);
	indices[indexOffset + 7] = glm::ivec3(positionOffset + 8, positionOffset + 2, positionOffset + 9);
	indices[indexOffset + 8] = glm::ivec3(positionOffset + 9, positionOffset + 2, positionOffset + 10);
}

void appendSphereLeaf(const glm::mat4 modelMatrix, Trees::TreeMesh &tm, int positionOffset, int indexOffset) {
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

		auto uv = sphereUvs[i];
		uv.x *= 0.3289;
		uv.y *= 0.4358;
		uv += glm::vec2(0.6711, 0);
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

void Trees::generateTree() {
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
		int indexOffset = leafIndicesOffset + i * trianglesPerLeaf;
#define USE_SPHERES_AS_LEAFS 1
#if USE_SPHERES_AS_LEAFS
		appendSphereLeaf(modelMatrix, tree_mesh, positionOffset, indexOffset);
#else
		appendLeaf(positionOffset, indexOffset, positions, normals, indices, modelMatrix);
#endif
	}
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
