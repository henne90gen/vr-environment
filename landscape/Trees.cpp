#include "Trees.h"

#include <array>
#include <random>

#include <Image.h>
#include <ImageOps.h>
#include <util/RenderUtils.h>

// FIXME shader
//  DEFINE_DEFAULT_SHADERS(landscape_Tree)
//  DEFINE_DEFAULT_SHADERS(landscape_Texture)
//  DEFINE_DEFAULT_SHADERS(landscape_FlatColor)
//  DEFINE_SHADER(landscape_NoiseLib)
//  DEFINE_SHADER(landscape_TreeComp)

void Trees::init() {
	// FIXME shader
	//  shader = CREATE_DEFAULT_SHADER(landscape_Tree);
	//  shader->attachShaderLib(SHADER_CODE(landscape_NoiseLib));

	initComputeShader();

	initModel();
	initGrid();

	barkTexture = std::make_shared<Texture>();

	Image img = {};
	if (ImageOps::load("./landscape_resources/assets/textures/bark_and_leafs_light.png", img)) {
		img.applyToTexture(barkTexture);
	}
}

void Trees::showGui() {
// FIXME imgui
#if 0
    ImGui::DragInt("Tree Count", &treeCount);
    ImGui::DragFloat("LOD Size", &lodSize);
    ImGui::DragFloat("LOD Inner Size", &lodInnerSize);
    ImGui::DragFloat("Grid Height", &gridHeight);
    ImGui::DragFloat("Global Tree Scale", &treeScale, 0.001F);
    treeSettings.showGui();
#endif
}

void Trees::render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
				   const TerrainParams &terrainParams) {
	if (!showTrees) {
		return;
	}

	// TODO(henne): compute shader execution can be moved into init
	renderComputeShader(terrainParams);

	if (usingGeneratedTrees) {
		generateTrees();
		renderTrees(projectionMatrix, viewMatrix, shaderToggles);
	} else {
		renderCubes(projectionMatrix, viewMatrix, shaderToggles);
		renderGrid(projectionMatrix, viewMatrix);
	}
}

void Trees::initComputeShader() {
	// FIXME shader
	//  compShader = std::make_shared<Shader>();
	//  compShader->attachComputeShader(SHADER_CODE(landscape_TreeComp));
	//  compShader->attachShaderLib(SHADER_CODE(landscape_NoiseLib));

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
}

void Trees::renderComputeShader(const TerrainParams &terrainParams) {
	compShader->bind();
	compShader->setUniform("treeCount", treeCount);
	compShader->setUniform("lodSize", lodSize);
	compShader->setUniform("lodInnerSize", lodInnerSize);
	terrainParams.setShaderUniforms(compShader);
	GL_Call(glBindImageTexture(0, treePositionTextureId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F));
	GL_Call(glDispatchCompute(4, 4, 1));
	GL_Call(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
}

void Trees::initGrid() {
	// FIXME shader
	//  flatColorShader = CREATE_DEFAULT_SHADER(landscape_FlatColor);
	gridVA = std::make_shared<VertexArray>(flatColorShader);
	std::vector<glm::vec3> vertices = {};
	vertices.emplace_back(0.0F, 0.0F, 0.0F);
	vertices.emplace_back(1.0F, 0.0F, 0.0F);
	vertices.emplace_back(1.0F, 0.0F, 1.0F);
	vertices.emplace_back(0.0F, 0.0F, 1.0F);

	BufferLayout layout = {{ShaderDataType::Float3, "a_Position"}};
	auto vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
	gridVA->addVertexBuffer(vertexBuffer);

	std::vector<unsigned int> indices = {};
	indices.emplace_back(0);
	indices.emplace_back(1);
	indices.emplace_back(1);
	indices.emplace_back(2);
	indices.emplace_back(2);
	indices.emplace_back(3);
	indices.emplace_back(3);
	indices.emplace_back(0);
	auto indexBuffer = std::make_shared<IndexBuffer>(indices);
	gridVA->setIndexBuffer(indexBuffer);
}

void Trees::renderGrid(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) {
	if (!showGrid) {
		return;
	}

	gridVA->bind();
	flatColorShader->bind();
	flatColorShader->setUniform("viewMatrix", viewMatrix);
	flatColorShader->setUniform("projectionMatrix", projectionMatrix);
	flatColorShader->setUniform("flatColor", glm::vec3(1.0F));

	const float lodH = lodSize / 2.0F;
	const float lodIH = lodInnerSize / 2.0F;
	const float smallSideLength = (lodSize - lodInnerSize) / 2.0F;
	std::vector<glm::vec4> gridOffsets = {
		  {lodInnerSize / -2.0F, lodInnerSize / -2.0F, lodInnerSize, lodInnerSize},

		  {-lodH, lodIH, smallSideLength, smallSideLength},
		  {-lodIH, lodIH, lodInnerSize, smallSideLength},
		  {lodIH, lodIH, smallSideLength, smallSideLength},

		  {-lodH, -lodIH, smallSideLength, lodInnerSize},
		  {lodIH, -lodIH, smallSideLength, lodInnerSize},

		  {-lodH, -lodH, smallSideLength, smallSideLength},
		  {-lodIH, -lodH, lodInnerSize, smallSideLength},
		  {lodIH, -lodH, smallSideLength, smallSideLength},
	};
	for (const auto &gridOffset : gridOffsets) {
		auto modelMatrix = glm::identity<glm::mat4>();
		modelMatrix = glm::translate(modelMatrix, glm::vec3(gridOffset.x, gridHeight, gridOffset.y));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(gridOffset.z, 1.0F, gridOffset.w));
		flatColorShader->setUniform("modelMatrix", modelMatrix);
		GL_Call(glDrawElements(GL_LINES, gridVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
	}
}

void Trees::initModel() { cubeVA = createCubeVA(shader); }

void Trees::renderCubes(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
						const ShaderToggles &shaderToggles) {
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

void Trees::generateTrees() {
	if (generatedTreesVA != nullptr) {
		for (auto &vb : generatedTreesVA->getVertexBuffers()) {
			const unsigned int glid = vb->getGLID();
			GL_Call(glDeleteBuffers(1, &glid));
		}
		generatedTreesVA->getVertexBuffers().clear();

		const unsigned int glid = generatedTreesVA->getIndexBuffer()->getGLID();
		GL_Call(glDeleteBuffers(1, &glid));
		generatedTreesVA->getIndexBuffer() = nullptr;
	} else {
		generatedTreesVA = std::make_shared<VertexArray>(shader);
	}

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
	generatedTreesVA->addVertexBuffer(vertexBuffer);

	auto indexBuffer = std::make_shared<IndexBuffer>(indices);
	generatedTreesVA->setIndexBuffer(indexBuffer);
}

void Trees::renderTrees(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix,
								 const ShaderToggles &shaderToggles) {
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
}