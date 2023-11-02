#pragma once
#include <glm/glm.hpp>
#include "uniforms.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "fragment.hpp"

struct Moon {
    glm::mat4 model;
    std::vector<Vertex> vertexArray;
    Fragment (*shader)(Fragment&);
    unsigned int radius;
    float orbitSpeed;
    float rotationSpeed;
    glm::vec3 translation;
    glm::vec3 scale;
    glm::vec3 rotation;

    void setOrbit(const unsigned int inRadius, const float inOrbitSpeed, const float inRotationSpeed) {
        radius = inRadius;
        orbitSpeed = inOrbitSpeed;
        rotationSpeed = inRotationSpeed;
    }

    void setTranslate(const float& x, const float& y, const float& z) {
        translation = glm::vec3(x, y, z);
    }

    void setOrbitTranslate(const float& time, const glm::vec3 parentTranslate) {
        translation = glm::vec3(parentTranslate.x + radius * sin(glm::radians(orbitSpeed * time)), 0.0f, parentTranslate.z + radius * cos(glm::radians(orbitSpeed * time)));
    }

    glm::vec3 calcLight(glm::vec3& L, const glm::vec3 parentTranslate) {
        glm::vec3 direction = glm::normalize(L - translation);
        glm::vec3 planetToMoon = glm::normalize(translation - parentTranslate);

        float angle = glm::degrees(acos(glm::dot(direction, planetToMoon)));

        float illuminationFactor = 1.0 - angle / 200.0;

        illuminationFactor = glm::clamp(illuminationFactor, 0.05f, 1.0f);

        return direction * illuminationFactor;
    }

    void setModel(const float& time, const glm::vec3 parentTranslate) {
        setOrbitTranslate(time, parentTranslate);
        model = glm::translate(glm::mat4(1.0f), translation);
        model = glm::rotate(model, glm::radians(rotationSpeed * time), rotation);
        model = glm::scale(model, scale);
    }

};

struct Planet {
    glm::mat4 model;
    std::vector<Vertex> vertexArray;
    Fragment (*shader)(Fragment&);
    unsigned int radius;
    float orbitSpeed;
    float rotationSpeed;
    glm::vec3 translation;
    glm::vec3 scale;
    glm::vec3 rotation;
    std::vector<Moon> moons;

    void setOrbit(const unsigned int inRadius, const float inOrbitSpeed, const float inRotationSpeed) {
        radius = inRadius;
        orbitSpeed = inOrbitSpeed;
        rotationSpeed = inRotationSpeed;
    }

    void setTranslate(const float& x, const float& y, const float& z) {
        translation = glm::vec3(x, y, z);
    }

    void setOrbitTranslate(const float& time) {
        translation = glm::vec3(radius * sin(glm::radians(orbitSpeed * time)), 0.0f, radius * cos(glm::radians(orbitSpeed * time)));
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

struct PlanetaryRing {
    glm::mat4 model;
    std::vector<Vertex> vertexArray;
    Fragment (*shader)(Fragment&);
    unsigned int radius;
    float orbitSpeed;
    glm::vec3 translation;
    glm::vec3 scale;

    void setOrbit(const unsigned int inRadius, const float inOrbitSpeed) {
        radius = inRadius;
        orbitSpeed = inOrbitSpeed;
    }

    void setTranslate(const float& x, const float& y, const float& z) {
        translation = glm::vec3(x, y, z);
    }

    void setOrbitTranslate(const float& time) {
        translation = glm::vec3(radius * sin(glm::radians(orbitSpeed * time)), 0.0f, radius * cos(glm::radians(orbitSpeed * time)));
    }

    glm::vec3 calcLight(glm::vec3& L) {
        return glm::normalize(translation- L);
    }

    void setModel(const float& time) {
        setOrbitTranslate(time);
        model = glm::translate(glm::mat4(1.0f), translation);
        model = glm::rotate(model, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
    }

};

