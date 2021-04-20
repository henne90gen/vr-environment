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

bool validate_framebuffer() {
	GLenum status = 0;
	GL_Call(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
	if (status == GL_FRAMEBUFFER_COMPLETE)
		return true;
	std::cerr << "Framebuffer is not complete: ";
	switch (status) {
	case GL_FRAMEBUFFER_UNDEFINED:
		std::cerr << "Could not define a framebuffer";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		std::cerr << "Some attachment is incomplete";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		std::cerr << "Some attachment is missing";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		std::cerr << "A draw buffer is incomplete";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		std::cerr << "A read buffer is incomplete";
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		std::cerr << "Framebuffers are not supported";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		std::cerr << "Multisample is incomplete";
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		std::cerr << "Layer targets are incomplete";
		break;
	default:
		std::cerr << "Unknown error";
		break;
	}

	std::cerr << std::endl;
	return false;
}
