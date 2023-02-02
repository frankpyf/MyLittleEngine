#pragma once
#include <glm/glm.hpp>

// Temp
namespace resource {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
    };

    struct Light
    {
        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
    };
}