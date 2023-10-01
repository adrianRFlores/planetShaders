#pragma once
#include <glm/glm.hpp>
#include "uniforms.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "fragment.hpp"

struct Planet {
    glm::mat4 model;
    std::vector<Vertex> vertexArray;
    Fragment (*shader)(Fragment&);
    unsigned int radius;
    float rotationSpeed;
};