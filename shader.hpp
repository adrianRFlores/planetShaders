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
glm::vec3 joolColor2(64.0f/255.0f, 79.0f/255.0f, 138.0f/255.0f);

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

    joolNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    joolNoise.SetSeed(1337);
    joolNoise.SetFrequency(0.025f);
    joolNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    joolNoise.SetFractalOctaves(5);
    joolNoise.SetFractalLacunarity(2.7f);
    joolNoise.SetFractalGain(0.4f);
    joolNoise.SetFractalWeightedStrength(0.2f);
    //sunNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Div);
    joolNoise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
    joolNoise.SetDomainWarpAmp(35.0f);

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
    float zoom = 1000.0f;

    float noise = joolNoise.GetNoise((pos.x + ox) * zoom, (pos.y + oy) * zoom, (pos.z + ox) * zoom);

    glm::vec3 tmpColor;

    if ( noise > 0.5f ) {
        tmpColor = joolColor1;
    }
    else if ( noise < 0.5f ) {
        tmpColor = joolColor2;
        noise = 1.0f - noise;
    } /*else {
        tmpColor = (joolColor1 + joolColor2) * 0.5f;
    }*/

    fragment.color = Color(
        tmpColor.x * 255.0f,
        tmpColor.y * 255.0f,
        tmpColor.z * 255.0f,
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