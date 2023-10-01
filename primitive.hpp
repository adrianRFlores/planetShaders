#pragma once
#include <glm/glm.hpp>
#include <vector>

std::vector<std::vector<Vertex>> primitiveAssembly(const std::vector<Vertex>& transformedVertices) {

    std::vector<std::vector<Vertex>> assembledVertices(transformedVertices.size() / 3);
    int count = 0;

    if(transformedVertices.size() % 3 == 0) {

        //#pragma omp parallel for
        for (int i = 0; i < transformedVertices.size(); i += 3) {
            std::vector<Vertex> triangle = {
                transformedVertices[i],
                transformedVertices[i + 1],
                transformedVertices[i + 2]
            };
            //assembledVertices.push_back(triangle);
            assembledVertices[count++] = triangle;
        }
    }

    return assembledVertices;
}