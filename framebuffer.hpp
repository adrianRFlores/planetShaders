#pragma once
#include <glm/glm.hpp>
#include <omp.h>
#include "color.hpp"
#include "fragment.hpp"
#include "FastNoiseLite.h"

const int FRAMEBUFFER_WIDTH = 800, FRAMEBUFFER_HEIGHT = 800;
const int FRAMEBUFFER_SIZE = FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT;

FastNoiseLite noiseGenerator;
const float scale = 1000.0f;

std::array<zFragment, FRAMEBUFFER_SIZE> framebuffer;

const zFragment star = {Color(100, 100, 100), 9999.0f};
const zFragment blank = {Color(0, 0, 0), 9999.0f};

void initStarNoise() {
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
}

void clear() {
    #pragma omp parallel for
    for (int i = 0; i < FRAMEBUFFER_SIZE; i++) {
        framebuffer[i] = zFragment{Color(0,0,0), 9999.0f};
    }
}

void clearFramebuffer(int ox, int oy) {

    #pragma omp parallel for
    for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
        for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
            float noiseValue = noiseGenerator.GetNoise((x + (ox * 100.0f)) * scale, (y + oy * 100.0f) * scale);

            // If the noise value is above a threshold, draw a star
            if (noiseValue > 0.995f) {
                framebuffer[y * FRAMEBUFFER_WIDTH + x] = star;
            } else {
                framebuffer[y * FRAMEBUFFER_WIDTH + x] = blank;
            }
        }
    }

}



void point(const Fragment& f) {
    if (f.z < framebuffer[f.pos.y * FRAMEBUFFER_WIDTH + f.pos.x].z) {
       framebuffer[f.pos.y * FRAMEBUFFER_WIDTH + f.pos.x] = zFragment{f.color, f.z};
    }
}