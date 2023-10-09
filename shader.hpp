#pragma once
#include <glm/glm.hpp>
#include "uniforms.hpp"
#include "fragment.hpp"
#include "FastNoiseLite.h"

FastNoiseLite sunNoise;
FastNoiseLite joolNoise;
FastNoiseLite laytheNoise;

glm::vec3 sunColor(115.0f/255.0f, 92.0f/255.0f, 221.0f/255.0f);
glm::vec3 joolColor1(170.0/255.0f, 89.0f/255.0f, 120.0f/255.0f);
glm::vec3 joolColor2(97.0f/255.0f, 231.0f/255.0f, 110.0f/255.0f);

void initNoise( ) {
    
    sunNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    sunNoise.SetSeed(1337);
    sunNoise.SetFrequency(0.04f);
    sunNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    sunNoise.SetFractalOctaves(5);
    sunNoise.SetFractalLacunarity(2.0f);
    sunNoise.SetFractalGain(0.4f);
    sunNoise.SetFractalWeightedStrength(0.6f);
    sunNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Div);

    joolNoise.SetNoiseType(FastNoiseLite::NoiseType_ValueCubic);
    joolNoise.SetSeed(1337);
    joolNoise.SetFrequency(0.02f);
    joolNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    joolNoise.SetFractalOctaves(5);
    joolNoise.SetFractalLacunarity(2.4f);
    joolNoise.SetFractalGain(0.5f);
    joolNoise.SetFractalWeightedStrength(0.2f);
    //sunNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Div);
    joolNoise.SetDomainWarpType(FastNoiseLite::DomainWarpType_BasicGrid);
    joolNoise.SetDomainWarpAmp(30.0f);

}

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

Fragment sunShader(Fragment& fragment) {

    glm::vec3 pos = fragment.originalPos;

    float ox = 1000.0f;
    float oy = 1000.0f;
    float zoom = 800.0f;

    float noise = sunNoise.GetNoise((pos.x + ox) * zoom, (pos.y + oy) * zoom, (pos.z + ox) * zoom);

    fragment.color = Color(
        sunColor.x * 255.0f * (1.0f - noise),
        sunColor.y * 255.0f * (1.0f - noise),
        sunColor.z * 255.0f * (1.0f - noise),
        fragment.color.a
    );

    return fragment;
}

Fragment joolShader(Fragment& fragment) {
    glm::vec3 pos = fragment.originalPos;

    float ox = 1000.0f;
    float oy = 1000.0f;
    float zoom = 800.0f;

    float noise = joolNoise.GetNoise((pos.x + ox) * zoom, (pos.y + oy) * zoom, (pos.z + ox) * zoom);

    glm::vec3 tmpColor = joolColor2 * (1.0f - noise);

    float noise2 = joolNoise.GetNoise((pos.x + ox + 1000) * zoom, (pos.y + oy + 1000) * zoom, (pos.z + ox + 1000) * zoom);

    glm::vec3 tmpColor2 = joolColor1 * (1.0f - noise) * 0.7f;

    fragment.color = Color(
        (tmpColor.x) * 255.0f * fragment.light,
        (tmpColor.z) * 255.0f * fragment.light,
        (tmpColor.y) * 255.0f * fragment.light,
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