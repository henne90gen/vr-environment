#pragma once

class Camera {
  public:
	glm::mat4 getViewMatrix() const {
		// FIXME this is only a stub -> fill in with correct code
		return glm::identity<glm::mat4>();
	}
	glm::mat4 getProjectionMatrix() const {
		// FIXME this is only a stub -> fill in with correct code
		return glm::identity<glm::mat4>();
	}
	glm::vec3 getPosition() const {
		// FIXME this is only a stub -> fill in with correct code
		return glm::vec3();
	}
	glm::vec3 getForwardDirection() const {
		// FIXME this is only a stub -> fill in with correct code
		return glm::vec3();
	}
	float getPitch() const {
		// FIXME this is only a stub -> fill in with correct code
		return 0.0F;
	}
	float getYaw() const {
		// FIXME this is only a stub -> fill in with correct code
		return 0.0F;
	}
	glm::quat getOrientation() const {
		// FIXME this is only a stub -> fill in with correct code
		return glm::quat();
	}
	void setViewportSize(float width, float height) {
		// FIXME this is only a stub -> fill in with correct code
	}
	void setFocalPoint(const glm::vec3 &newFocalPoint) {
		// FIXME this is only a stub -> fill in with correct code
	}
};
