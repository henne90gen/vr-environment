#pragma once

#include <vector>
#include <glm/glm.hpp>

/**
 * Copied from http://www.songho.ca/opengl/gl_sphere.html
 * Creates a unit sphere with center at (0,0,0) and radius 1.0
 * @param sectorCount "rings" around the sphere
 * @param stackCount "strips" from top to bottom of the sphere
 */
struct Sphere {
    int sectorCount = 36;
    int stackCount = 18;

    [[nodiscard]] int vertexCount() const { return (sectorCount + 1) * (stackCount + 1); }
    [[nodiscard]] int indexCount() const { return (stackCount - 1) * sectorCount * 2; }
    void append(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs,
                std::vector<glm::ivec3> &indices) const;
};
