#include "OpenGLUtils.h"

#include <iostream>
#include <vector>

void GL_ClearError() {
	while (glGetError() != GL_NO_ERROR) {}
}

bool GL_LogCall(const char *function, const char *file, int line) {
	bool noErrors = true;
	while (GLenum error = glGetError() != GL_NO_ERROR) {
		std::cout << "OpenGL error [0x" << std::hex << error << std::dec << "]: " << file << "/" << function << ": "
				  << line << std::endl;
		noErrors = false;
	}
	return noErrors;
}

glm::mat4 createViewMatrix(const glm::vec3 &cameraPosition, const glm::vec3 &cameraRotation) {
	auto viewMatrix = glm::identity<glm::mat4>();
	viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
	viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
	viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
	viewMatrix = glm::translate(viewMatrix, cameraPosition);
	return viewMatrix;
}

glm::mat4 createModelMatrix(const glm::vec3 &modelPosition, const glm::vec3 &modelRotation,
							const glm::vec3 &modelScale) {
	auto modelMatrix = glm::identity<glm::mat4>();
	modelMatrix = glm::translate(modelMatrix, modelPosition);
	modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
	modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
	modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
	modelMatrix = glm::scale(modelMatrix, modelScale);
	return modelMatrix;
}
