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
    glm::vec3 translation;
    glm::vec3 scale;
    glm::vec3 rotation;

    void setOrbit(const unsigned int inRadius, const float inRotationSpeed) {
        radius = inRadius;
        rotationSpeed = inRotationSpeed;
    }

    void setTranslate(const float& x, const float& y, const float& z) {
        translation = glm::vec3(x, y, z);
    }

    void setOrbitTranslate(const float& time) {
        translation = glm::vec3(radius * sin(glm::radians(rotationSpeed * time)), 0.0f, radius * cos(glm::radians(rotationSpeed * time)));
    }

    glm::vec3 calcLight(glm::vec3& L) {
        return glm::normalize(L - translation);
    }

    void setModel(const float& time) {
        setOrbitTranslate(time);
        model = glm::translate(glm::mat4(1.0f), translation);
        model = glm::rotate(model, glm::radians(rotationSpeed * time), rotation);
        model = glm::scale(model, scale);
    }

};

