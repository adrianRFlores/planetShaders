#pragma once
#include <glm/glm.hpp>
#include <cmath>
#include "uniforms.hpp"
#include "fragment.hpp"
#include "FastNoiseLite.h"

//Se supone que me iba a basar en los planetas de KSP, pero hubo un cambio de planes en el camino
FastNoiseLite sunNoise;
FastNoiseLite joolNoise;
FastNoiseLite joolNoise2;
FastNoiseLite laytheNoise;
FastNoiseLite cityNoise;

glm::vec3 sunColor(115.0f/255.0f, 92.0f/255.0f, 221.0f/255.0f);
glm::vec3 joolColor1(99.0f/255.0f, 76.0f/255.0f, 58.0f/255.0f);
glm::vec3 sandColor(201.0f/255.0f, 179.0f/255.0f, 122.0f/255.0f);
glm::vec3 joolColor2(47.0f/255.0f, 125.0f/255.0f, 210.0f/255.0f);
glm::vec3 dwarfColor(189.0f/255.0f, 151.0f/255.0f, 117.0f/255.0f);
glm::vec3 dwarfColorBasin(103.0f/255.0f, 80.0f/255.0f, 62.0f/255.0f);

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
    joolNoise.SetDomainWarpType(FastNoiseLite::DomainWarpType_BasicGrid);
    joolNoise.SetDomainWarpAmp(30.0f);

    joolNoise2.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    joolNoise2.SetSeed(1337);
    joolNoise2.SetFrequency(0.01f);

    laytheNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    laytheNoise.SetFrequency(0.01f);
    laytheNoise.SetFractalOctaves(6);

    cityNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    cityNoise.SetFrequency(0.03f);
    cityNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    cityNoise.SetFractalOctaves(6);
    cityNoise.SetFractalLacunarity(2.5f);
    cityNoise.SetFractalGain(0.0f);
    cityNoise.SetFractalWeightedStrength(0.0f);
    cityNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    cityNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Mul);
    cityNoise.SetCellularJitter(0.9f);
    cityNoise.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
    cityNoise.SetDomainWarpAmp(55.0f);

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

    float noise = joolNoise.GetNoise((pos.x + ox) * zoom * 0.8f, (pos.y + oy) * zoom * 0.8f, (pos.z + ox) * zoom * 0.8f);

    glm::vec3 tmpColor = joolColor2 * (1.0f - noise);

    float noise2 = joolNoise2.GetNoise((pos.x + ox + 1000) * zoom, (pos.y + oy + 1000) * zoom, (pos.z + ox + 1000) * zoom);

    if ( fragment.light < 0.05f && noise2 > 0.25f) {
        //amarillo
        tmpColor = glm::vec3(221.0f/255.0f, 171.0f/255.0f, 91.0f/255.0f);

        //verde
        //tmpColor = glm::vec3(125.0f/255.0f, 186.0f/255.0f, 116.0f/255.0f);

        float cityLightNoise = cityNoise.GetNoise((pos.x + ox + 5000) * zoom, (pos.y + oy + 5000) * zoom, (pos.z + ox + 6000) * zoom);

        if(1.0f - fabs(cityLightNoise) > 0.2f) {
            float normalizedNoise = ((1.0f - fabs(cityLightNoise)) - 0.2f)/0.8f;
            fragment.color = Color(
                tmpColor.x * 255.0f * normalizedNoise,
                tmpColor.y * 255.0f * normalizedNoise,
                tmpColor.z * 255.0f * normalizedNoise,
                fragment.color.a
            );
        } else {
            fragment.color = Color(
                0, 0, 0,
                fragment.color.a
            );
        }
        return fragment;

    }

    if ( noise2 > 0.25f) {
        float normalizedNoise = (noise2 - 0.3f) / (0.7f);
        float t;
        if ( normalizedNoise < 0.05 ){
            t = normalizedNoise / 0.05f;
            tmpColor = (1.0f - t) * sandColor + t * joolColor1;
        } else {
            tmpColor = joolColor1;
        }

    }

    float noise3 = joolNoise2.GetNoise((pos.x + ox + 5000) * zoom * 0.9f, (pos.y + oy + 5000) * zoom * 0.9f, (pos.z + ox + 5000) * zoom * 0.9f);
    if(noise3 > 0.4f) {
        tmpColor = glm::vec3(1.0f);
    }

    fragment.color = Color(
        (tmpColor.x) * 255.0f * fragment.light,
        (tmpColor.y) * 255.0f * fragment.light,
        (tmpColor.z) * 255.0f * fragment.light,
        fragment.color.a
    );

    return fragment;

}

Fragment laytheShader(Fragment& fragment) {

    glm::vec3 pos = fragment.originalPos;

    float ox = 1000.0f;
    float oy = 1000.0f;
    float zoom = 800.0f;

    float noise = laytheNoise.GetNoise((pos.x + ox) * zoom * 0.8f, (pos.y + oy) * zoom * 0.8f, (pos.z + ox + 5000) * zoom * 0.8f);

    glm::vec3 tmpColor = noise > 0.2f ? dwarfColorBasin * noise / 0.2f : dwarfColor * 0.7f * (1.0f - noise);

    float volcanoNoise = laytheNoise.GetNoise((pos.x + ox + 5000) * zoom, (pos.y + oy + 5000) * zoom, (pos.z + ox + 6000) * zoom);

    float cityLightNoise = cityNoise.GetNoise((pos.x + ox + 5000), (pos.y + oy + 5000), (pos.z + ox + 6000));

    if ( volcanoNoise > 0.7f && noise < 0.2f) tmpColor = glm::vec3(240.0f/255.0f, 50.0f/255.0f, 50.0f/255.0f);

    if (cityLightNoise > 0.8f) {
        tmpColor = glm::vec3(120.0f/255.0f);
    }

    fragment.color = Color(
        (tmpColor.x) * 255.0f * fragment.light,
        (tmpColor.y) * 255.0f * fragment.light,
        (tmpColor.z) * 255.0f * fragment.light,
        fragment.color.a
    );

    return fragment;
}

Fragment ringShader(Fragment& fragment) {
    fragment.color = Color(
        fragment.color.r * fragment.light,
        fragment.color.g * fragment.light,
        fragment.color.b * fragment.light,
        fragment.color.a
    );

    return fragment;
}

Fragment testShader(Fragment& fragment) {

    glm::vec3 pos = fragment.originalPos;

    float ox = 1000.0f;
    float oy = 1000.0f;
    float zoom = 800.0f;

    glm::vec3 tmpColor(1.0f, 186.0f/255.0f, 73.0f/255.0f);

    float cityLightNoise = cityNoise.GetNoise((pos.x + ox + 5000) * zoom, (pos.y + oy + 5000) * zoom, (pos.z + ox + 6000) * zoom);

    fragment.color = Color(
        tmpColor.x * 255.0f * (1.0f - fabs(cityLightNoise)),
        tmpColor.y * 255.0f * (1.0f - fabs(cityLightNoise)),
        tmpColor.z * 255.0f * (1.0f - fabs(cityLightNoise)),
        fragment.color.a
    );

    return fragment;
}