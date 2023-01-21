#pragma once
#include <glm/glm.hpp>

// Temp
namespace resource {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
    };
    
    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
}