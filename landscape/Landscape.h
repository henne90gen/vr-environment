#pragma once

#include <functional>
#include <gl/IndexBuffer.h>
#include <gl/Texture.h>
#include <gl/VertexArray.h>
#include <memory>

#include "Camera.h"
#include "Layers.h"
#include "ShaderToggles.h"
#include "Terrain.h"

struct Light {
	glm::vec3 fragmentToLightDir = glm::vec3(0.0F, 1.0F, 0.0F);
	glm::vec3 color = glm::vec3(1.0F);

	float ambient = 0.1F;
	float diffuse = 1.0F;
	float specular = 0.3F;

	float distance = 100.0F;
	float scale = 10.0F;
};

class Landscape {
  public:
	Landscape() = default;
	~Landscape() = default;

	void setup();
	void tick();
	void destroy();
	void onAspectRatioChange();

  private:
	std::shared_ptr<VertexArray> quadVA;
	std::shared_ptr<VertexArray> cubeVA;

	std::shared_ptr<Shader> flatShader;
	std::shared_ptr<Shader> textureShader;
	std::shared_ptr<Shader> lightingShader;
	std::shared_ptr<Shader> ssaoShader;
	std::shared_ptr<Shader> ssaoBlurShader;

	GLuint gBuffer = 0;
	GLuint gPosition = 0;
	GLuint gNormal = 0;
	GLuint gAlbedo = 0;
	GLuint gDoLighting = 0;
	GLuint depthBuffer = 0;

	GLuint ssaoFbo = 0;
	GLuint ssaoColorBuffer = 0;

	GLuint ssaoBlurFbo = 0;
	GLuint ssaoColorBlurBuffer = 0;

	std::vector<glm::vec3> ssaoKernel = {};
	unsigned int ssaoNoiseTexture = 0;

	Camera playerCamera = {};
	Terrain terrain = {};
	glm::vec3 atmosphere = glm::vec3(0.4F, 0.45F, 1.2F);

	void renderTerrain(const Camera &camera, const Light &light, const ShaderToggles &shaderToggles);
	void renderLight(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const Light &light);
	void renderSSAO();
	void renderSSAOBlur();
	void renderGBufferToQuad(const Camera &camera, const Light &light, const ShaderToggles &shaderToggles);
	void renderGBufferViewer();

	void renderTextureViewer();
	void renderTexture(const glm::vec3 &texturePosition, float zoom, unsigned int texture);

	void initGBuffer();
	void initSSAOBuffer();
	void initSSAOBlurBuffer();
	void initKernelAndNoiseTexture();

  public:
	// FIXME stubs
	float getAspectRatio() const {
		// FIXME this is only a stub -> fill in with correct code
		return 0.0F;
	}
	unsigned int getWidth() const {
		// FIXME this is only a stub -> fill in with correct code
		return 0;
	}
	unsigned int getHeight() const {
		// FIXME this is only a stub -> fill in with correct code
		return 0;
	}
	double getLastFrameTime() const {
		// FIXME this is only a stub -> fill in with correct code
		return 0.0F;
	}
	Camera getCamera() {
		// FIXME this is only a stub -> fill in with correct code
		return {};
	}
};
