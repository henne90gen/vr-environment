#include "Trees.h"

#include <array>
#include <random>

#include <Image.h>
#include <ImageOps.h>
#include <util/RenderUtils.h>

#include <cgv_gl/box_wire_renderer.h>

#include "../macros.h"

// FIXME shader
//  DEFINE_DEFAULT_SHADERS(landscape_Tree)
//  DEFINE_DEFAULT_SHADERS(landscape_Texture)
//  DEFINE_DEFAULT_SHADERS(landscape_FlatColor)
//  DEFINE_SHADER(landscape_NoiseLib)
//  DEFINE_SHADER(landscape_TreeComp)

bool Trees::init(cgv::render::context &ctx) {
	auto &box_wire = ref_box_wire_renderer(ctx, 1);
	if (!box_wire.init(ctx)) {
		return false;
	}

	initGrid();

	if (!initComputeShader(ctx)) {
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
		renderTrees(ctx, shaderToggles);
	}
}

bool Trees::initComputeShader(cgv::render::context &ctx) {
	if (!tree_placement_compute_shader.is_created()) {
		if (!tree_placement_compute_shader.build_program(ctx, "tree_placement.glpr", true)) {
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
#if 0
	GL_Call(glGenTextures(1, &treePositionTextureId));
	GL_Call(glActiveTexture(GL_TEXTURE0));
	GL_Call(glBindTexture(GL_TEXTURE_2D, treePositionTextureId));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, treePositionTextureWidth, treePositionTextureHeight, 0, GL_RGBA,
						 GL_FLOAT, nullptr));
	GL_Call(glBindImageTexture(0, treePositionTextureId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F));
#endif
}

void Trees::renderComputeShader(cgv::render::context &ctx, const TerrainParams &terrainParams) {
#if 0
	compShader->bind();
	compShader->setUniform("treeCount", treeCount);
	compShader->setUniform("lodSize", lodSize);
	compShader->setUniform("lodInnerSize", lodInnerSize);
	terrainParams.setShaderUniforms(compShader);
	GL_Call(glBindImageTexture(0, treePositionTextureId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F));
	GL_Call(glDispatchCompute(4, 4, 1));
	GL_Call(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
#else
	tree_placement_compute_shader.set_uniform(ctx, "treeCount", treeCount);
	tree_placement_compute_shader.set_uniform(ctx, "lodSize", lodSize);
	tree_placement_compute_shader.set_uniform(ctx, "lodInnerSize", lodInnerSize);
	// TODO set terrainParams to uniforms
	GL_Call(glBindImageTexture(0, get_gl_id(tree_position_texture.handle), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F));
	GL_Call(glDispatchCompute(4, 4, 1));
	GL_Call(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
#endif
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

void appendSphereLeaf(const glm::mat4 modelMatrix, std::vector<glm::vec3> &positions, std::vector<glm::vec3> &normals,
					  std::vector<glm::vec2> &uvs, std::vector<glm::ivec3> &indices, int positionOffset,
					  int indexOffset) {
	std::vector<glm::vec3> vertices = {};
	std::vector<glm::vec3> sphereNormals = {};
	std::vector<glm::vec2> sphereUvs = {};
	std::vector<glm::ivec3> sphereIndices = {};

	Sphere s = {5, 3};
	s.append(vertices, sphereNormals, sphereUvs, sphereIndices);

	for (int i = 0; i < vertices.size(); i++) {
		const auto &vertex = vertices[i];
		positions[positionOffset + i] = glm::vec3(modelMatrix * glm::vec4(vertex, 1.0));

		const auto &normal = sphereNormals[i];
		normals[positionOffset + i] = glm::vec3(modelMatrix * glm::vec4(normal, 0.0F));

		auto uv = sphereUvs[i];
		uv.x *= 0.3289;
		uv.y *= 0.4358;
		uv += glm::vec2(0.6711, 0);
		uvs[positionOffset + i] = uv;
	}

	for (int i = 0; i < sphereIndices.size(); i++) {
		indices[indexOffset + i] = sphereIndices[i] + positionOffset;
	}
}

void Trees::generateTree() {
	std::vector<glm::vec3> positions = {};
	std::vector<glm::vec3> normals = {};
	std::vector<glm::vec2> uvs = {};
	std::vector<glm::ivec3> indices = {};

	Tree *tree = Tree::create(treeSettings);
	tree->construct(positions, normals, uvs, indices);

	// adjust uv coordinates to be constrained to the bark side of the texture
#pragma omp parallel for
	for (int i = 0; i < uvs.size(); i++) {
		uvs[i].x *= 0.671;
	}

	std::vector<glm::mat4> leafModelMatrices = {};
	tree->addLeaves(leafModelMatrices);

	int leafPositionOffset = positions.size();
	int leafIndicesOffset = indices.size();

	int sectorCount = 7;
	int stackCount = 5;
	int verticesPerLeaf = (sectorCount + 1) * (stackCount + 1);
	int indicesPerLeaf = (stackCount - 1) * sectorCount * 2;
	int totalVertices = leafPositionOffset + static_cast<int>(leafModelMatrices.size()) * verticesPerLeaf;
	int totalIndices = leafIndicesOffset + static_cast<int>(leafModelMatrices.size()) * indicesPerLeaf;

	positions.resize(totalVertices);
	normals.resize(totalVertices);
	uvs.resize(totalVertices);
	indices.resize(totalIndices);

#pragma omp parallel for
	for (int i = 0; i < leafModelMatrices.size(); i++) {
		const auto &modelMatrix = leafModelMatrices[i];
		int positionOffset = leafPositionOffset + i * verticesPerLeaf;
		int indexOffset = leafIndicesOffset + i * indicesPerLeaf;
#define USE_SPHERES_AS_LEAFS 1
#if USE_SPHERES_AS_LEAFS
		appendSphereLeaf(modelMatrix, positions, normals, uvs, indices, positionOffset, indexOffset);
#else
		appendLeaf(positionOffset, indexOffset, positions, normals, indices, modelMatrix);
#endif
	}

	auto vertexData = std::vector<float>(positions.size() * 8);
#pragma omp parallel for
	for (int i = 0; i < positions.size(); i++) {
		vertexData[i * 8 + 0] = positions[i].x;
		vertexData[i * 8 + 1] = positions[i].y;
		vertexData[i * 8 + 2] = positions[i].z;
		vertexData[i * 8 + 3] = normals[i].x;
		vertexData[i * 8 + 4] = normals[i].y;
		vertexData[i * 8 + 5] = normals[i].z;
		vertexData[i * 8 + 6] = uvs[i].x;
		vertexData[i * 8 + 7] = uvs[i].y;
	}

	BufferLayout layout = {
		  {ShaderDataType::Float3, "a_Position"},
		  {ShaderDataType::Float3, "a_Normal"},
		  {ShaderDataType::Float2, "a_UV"},
	};
	auto vertexBuffer = std::make_shared<VertexBuffer>(vertexData, layout);
	// FIXME generatedTreesVA->addVertexBuffer(vertexBuffer);

	auto indexBuffer = std::make_shared<IndexBuffer>(indices);
	// FIXME generatedTreesVA->setIndexBuffer(indexBuffer);
}

void Trees::renderTrees(cgv::render::context &ctx, const ShaderToggles &shaderToggles) {
	// FIXME use custom renderer here

#if 0
	generatedTreesVA->bind();
	shader->bind();
	generatedTreesVA->setShader(shader);
	auto modelMatrix = glm::identity<glm::mat4>();
	modelMatrix = glm::scale(modelMatrix, glm::vec3(treeScale));
	auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(viewMatrix * modelMatrix)));
	shader->setUniform("modelMatrix", modelMatrix);
	shader->setUniform("viewMatrix", viewMatrix);
	shader->setUniform("projectionMatrix", projectionMatrix);
	shader->setUniform("normalMatrix", normalMatrix);
	shader->setUniform("flatColor", glm::vec3(1.0F, 0.0F, 0.0F));
	shader->setUniform("textureSampler", 0);
	shader->setUniform("positionTexture", 1);

	GL_Call(glActiveTexture(GL_TEXTURE0));
	barkTexture->bind();

	GL_Call(glActiveTexture(GL_TEXTURE1));
	GL_Call(glBindTexture(GL_TEXTURE_2D, treePositionTextureId));

	if (shaderToggles.drawWireframe) {
		GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
	}

#if 1
	GL_Call(glDrawElementsInstanced(GL_TRIANGLES, generatedTreesVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT,
									nullptr, treeCount));
#else
	GL_Call(glDrawElements(GL_TRIANGLES, generatedTreesVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
#endif

	if (shaderToggles.drawWireframe) {
		GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	}
#endif
}

void Trees::clear(cgv::render::context &ctx) { ref_box_wire_renderer(ctx, -1); }

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
