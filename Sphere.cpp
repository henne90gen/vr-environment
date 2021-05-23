#include "Sphere.h"

#include <glm/ext.hpp>

void Sphere::append(std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs,
                    std::vector<glm::ivec3> &indices) const {
    float sectorStep = glm::two_pi<float>() / static_cast<float>(sectorCount);
    float stackStep = glm::pi<float>() / static_cast<float>(stackCount);

    for (int i = 0; i <= stackCount; i++) {
        float stackAngle = glm::half_pi<float>() - static_cast<float>(i) * stackStep; // starting from pi/2 to -pi/2
        float xy = glm::cos(stackAngle);
        float z = glm::sin(stackAngle);

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; j++) {
            float sectorAngle = static_cast<float>(j) * sectorStep;
            float x = xy * glm::cos(sectorAngle);
            float y = xy * glm::sin(sectorAngle);
            vertices.emplace_back(x, y, z);

            // center of sphere is (0,0,0), thus the vertex coordinate is also the normal
            normals.emplace_back(x, y, z);

            float s = static_cast<float>(j) / static_cast<float>(sectorCount);
            float t = static_cast<float>(i) / static_cast<float>(stackCount);
            uvs.emplace_back(s, t);
        }
    }

    // stackCount * sectorCount * 2 - sectorCount * 2
    // (stackCount - 1) * sectorCount * 2

    int k1 = 0;
    int k2 = 0;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1); // beginning of current stack
        k2 = k1 + sectorCount + 1;  // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0) {
                indices.emplace_back(k1, k2, k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1)) {
                indices.emplace_back(k1 + 1, k2, k2 + 1);
            }
        }
    }
}
