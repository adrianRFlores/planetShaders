#pragma once
#include <glm/glm.hpp>
#include "uniforms.hpp"
#include "fragment.hpp"
#include "FastNoiseLite.h"

Vertex vertexShaderPlanet(const Vertex& vertex, const Uniforms& uniforms, const glm::mat4& model) {

    glm::vec4 clipSpaceVertex = uniforms.projection * uniforms.view * model * glm::vec4(vertex.pos, 1.0f);

    glm::vec3 ndcVertex = glm::vec3(clipSpaceVertex) / clipSpaceVertex.w;

    glm::vec4 screenVertex = uniforms.viewport * glm::vec4(ndcVertex, 1.0f);
    
    glm::vec3 transformedNormal = glm::normalize(glm::mat3(model) * vertex.normal);

    glm::vec3 transformedWorldPos = glm::vec3(model * glm::vec4(vertex.pos, 1.0f));

    return Vertex{
        glm::vec3(screenVertex),
        transformedNormal,
        vertex.tex,
        transformedWorldPos,
        vertex.pos
    };
}

Fragment fragmentShader(Fragment& fragment) {
    fragment.color = Color(
        fragment.color.r * fragment.light,
        fragment.color.g * fragment.light,
        fragment.color.b * fragment.light,
        fragment.color.a
    );

    return fragment;
}

Fragment tierra(Fragment& fragment) {
    Color color;

    glm::vec3 groundColor = glm::vec3(0.44f, 0.51f, 0.33f);
    glm::vec3 oceanColor = glm::vec3(0.12f, 0.38f, 0.57f);
    glm::vec3 cloudColor = glm::vec3(1.0f, 1.0f, 1.0f);

    float x = fragment.originalPos.x;
    float y = fragment.originalPos.y;
    float z = fragment.originalPos.z;
    /* glm::vec2 uv = glm::vec2(fragment.originalPos.x, fragment.originalPos.y); */
    float radius = sqrt(x*x + y*y + z*z);

    /* glm::vec2 uv = glm::vec2( */
    /*     atan2(x, z), */
    /*     acos(y/sqrt(x*x + y*y + z*z)) */
    /* ); */

    glm::vec3 uv = glm::vec3(
        atan2(x, z),
        acos(y / radius),
        radius
    );

    glm::vec3 uv2 = glm::vec3(
        atan2(x + 10, z),
        acos(y / radius),
        radius
    );


    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    /* noiseGenerator.SetRotationType3D(FastNoiseLite::RotationType3D_ImproveXYPlanes); */
    /* noiseGenerator.DomainWarp(uv.x, uv.y, uv.z); */

    float ox = 1200.0f;
    float oy = 3000.0f;
    float zoom = 100.0f;

    /* float noiseValue1 = noiseGenerator.GetNoise(uv.x * zoom, uv.y * zoom); */
    /* float noiseValue2 = noiseGenerator.GetNoise(uv.y * zoom + 1000.0f, uv.x * zoom + 1000.0f); */
    /* float noiseValue = (noiseValue1 + noiseValue2) * 0.5f; */

    float noiseValue1 = noiseGenerator.GetNoise(uv.x * zoom, uv.y * zoom, uv.z * zoom);
    float noiseValue2 = noiseGenerator.GetNoise(uv2.x * zoom + ox, uv2.y * zoom, uv2.z * zoom + oy);
    float noiseValue = (noiseValue1 + noiseValue2) * 0.5f;



    glm::vec3 tmpColor = (noiseValue < 0.2f) ? oceanColor : groundColor;

    float oxc = 5500.0f;
    float oyc = 6900.0f;
    float zoomc = 300.0f;

    float noiseValueC = noiseGenerator.GetNoise((uv.x + oxc) * zoomc, (uv.y + oyc) * zoomc);
    
    if (noiseValueC > 0.5f) {
        tmpColor = cloudColor;
    }


    color = Color(tmpColor.x * 255, tmpColor.y * 255, tmpColor.z * 255);

    

    fragment.color = Color(color.r * fragment.light, color.g * fragment.light, color.b * fragment.light);

    return fragment;
}

Fragment sunShader(Fragment& fragment) {
    fragment.color = Color(
        fragment.color.r,
        fragment.color.g,
        fragment.color.b,
        fragment.color.a
    );

    return fragment;
}

Fragment joolShader(Fragment& fragment) {
    fragment.color = Color(
        fragment.color.r * fragment.light,
        fragment.color.g * fragment.light,
        fragment.color.b * fragment.light,
        fragment.color.a
    );

    return fragment;
}

Fragment laytheShader(Fragment& fragment) {
    fragment.color = Color(
        fragment.color.r * fragment.light,
        fragment.color.g * fragment.light,
        fragment.color.b * fragment.light,
        fragment.color.a
    );

    return fragment;
}