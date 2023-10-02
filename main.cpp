#include <iostream>
#include <math.h>
#include <algorithm>
#include <SDL2/SDL.h>
#include <omp.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "loadOBJ.hpp"
#include "color.hpp"
#include "framebuffer.hpp"
#include "uniforms.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "primitive.hpp"
#include "planets.hpp"

std::vector<glm::vec3> vec3Array;

glm::vec3 L(0.0f, 0.0f, 0.0f);

const int cameraRadius = 1;
const float angle = 0.00005f;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error: Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("SR2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    return true;
}

void renderBuffer(SDL_Renderer* renderer) {
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);

    void* texturePixels;
    int pitch;
    SDL_LockTexture(texture, NULL, &texturePixels, &pitch);

    Uint32 format = SDL_PIXELFORMAT_ARGB8888;
    SDL_PixelFormat* mappingFormat = SDL_AllocFormat(format);

    Uint32* texturePixels32 = static_cast<Uint32*>(texturePixels);
    #pragma omp parallel for
    for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
        for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
            int framebufferY = FRAMEBUFFER_HEIGHT - y - 1;  // Reverse the order of rows
            int index = y * (pitch / sizeof(Uint32)) + x;
            const Color& color = framebuffer[framebufferY * FRAMEBUFFER_WIDTH + x].color;
            texturePixels32[index] = SDL_MapRGBA(mappingFormat, color.r, color.g, color.b, color.a);
        }
    }

    SDL_UnlockTexture(texture);
    SDL_Rect textureRect = {0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT};
    SDL_RenderCopy(renderer, texture, NULL, &textureRect);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);
}

std::pair<float, float> barycentricCoordinates(const glm::ivec2& P, const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
    glm::vec3 bary = glm::cross(
        glm::vec3(C.x - A.x, B.x - A.x, A.x - P.x),
        glm::vec3(C.y - A.y, B.y - A.y, A.y - P.y)
    );

    /*if (abs(bary.z) < 1) {
        return std::make_pair(-1, -1);
    }*/

    return std::make_pair(
        bary.y / bary.z,
        bary.x / bary.z
    );    
}

std::vector<Fragment> filledTriangle(const Vertex& a, const Vertex& b, const Vertex& c, std::vector<Fragment>& fragments) {
  //std::vector<Fragment> fragments;
  glm::vec3 A = a.pos;
  glm::vec3 B = b.pos;
  glm::vec3 C = c.pos;

  float minX = std::min(std::min(A.x, B.x), C.x);
  float minY = std::min(std::min(A.y, B.y), C.y);
  float maxX = std::max(std::max(A.x, B.x), C.x);
  float maxY = std::max(std::max(A.y, B.y), C.y);

  for (int y = static_cast<int>(std::ceil(minY)); y <= static_cast<int>(std::floor(maxY)); ++y) {
    for (int x = static_cast<int>(std::ceil(minX)); x <= static_cast<int>(std::floor(maxX)); ++x) {
      if (x < 0 || y < 0 || y > FRAMEBUFFER_HEIGHT || x > FRAMEBUFFER_WIDTH)
        continue;
        
      glm::ivec2 P(x, y);
      auto barycentric = barycentricCoordinates(P, A, B, C);
      float w = 1 - barycentric.first - barycentric.second;
      float v = barycentric.first;
      float u = barycentric.second;
      float epsilon = 1e-10;

      if (w < epsilon || v < epsilon || u < epsilon)
        continue;
          
      //double z = A.z * w + B.z * v + C.z * u;
      /*
       glm::vec3 normal = glm::normalize( 
           a.normal * w + b.normal * v + c.normal * u
       );*/

      glm::vec3 lightDirection = glm::normalize( L - glm::vec3(w, v, u) );

      float intensity = glm::dot( glm::normalize( a.normal * w + b.normal * v + c.normal * u), lightDirection);
      
      glm::vec3 worldPos = a.worldPos * w + b.worldPos * v + c.worldPos * u;
      glm::vec3 originalPos = a.originalPos * w + b.originalPos * v + c.originalPos * u;

      fragments.push_back(
        Fragment{
          P,
          worldPos,
          originalPos,
          A.z * w + B.z * v + C.z * u,
          Color(255, 255, 255),
          intensity}
      );
    }
}
  //return fragments;
}

glm::vec3 getCenter(std::vector<Vertex> transformedVertices, std::vector<float> boundingBox){
    float maxX = 0, maxY = 0, maxZ = 0;
    float minX = 0, minY = 0, minZ = 0;

    for(const Vertex& vert : transformedVertices) {
        if(vert.pos.x > maxX) {
            maxX = vert.pos.x;
        } else if(vert.pos.x < minX) minX = vert.pos.x;

        if(vert.pos.y > maxY) {
            maxY = vert.pos.y;
        } else if(vert.pos.y < minY) minY = vert.pos.y;

        if(vert.pos.z > maxZ) {
            maxZ = vert.pos.z;
        } else if(vert.pos.z < minZ) minZ = vert.pos.z;
    }

    return glm::vec3(maxX + minX, maxY + minY, maxZ + minZ) * 0.5f;
}

void rasterizePlanet(const std::vector<std::vector<Vertex>>& assembledVertices, const Uniforms& uniforms, const glm::mat4& model, std::vector<Fragment>& fragments) {

    for (std::vector<Vertex> triangleVertices : assembledVertices) {
        filledTriangle(triangleVertices[0], triangleVertices[1], triangleVertices[2], fragments);
    }

}

void renderPlanet(const Planet& planet, const Uniforms& uniforms) {

    std::vector<Vertex> transformedVertices(planet.vertexArray.size());

    #pragma omp parallel for
    for (int i = 0; i<planet.vertexArray.size(); i++){
        transformedVertices[i] = vertexShaderPlanet(planet.vertexArray[i], uniforms, planet.model);
    }
    
    std::vector<std::vector<Vertex>> assembledVertices = primitiveAssembly(transformedVertices);

    std::vector<Fragment> fragments;
    rasterizePlanet(assembledVertices, uniforms, planet.model, fragments);

    for (int i = 0; i<fragments.size(); i++){
        const Fragment& frag = planet.shader(fragments[i]);
        point(frag);
    }

}

int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_EVERYTHING);

    init();

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tex; //future-proofing
    std::vector<Face> faces;
    Uniforms uniforms;

    bool loaded = loadOBJ("sphere.obj", vertices, faces, normals, tex);
    std::vector<Vertex> vertexArray = setupVertexArray(vertices, faces, normals);

    glm::mat4 model = glm::mat4(1);
    glm::mat4 view = glm::mat4(1);
    glm::mat4 projection = glm::mat4(1);

    uniforms.model = model,
    uniforms.view = view;

    glm::vec3 translationVector(0.0f, 0.0f, 0.0f);
    glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
    glm::vec3 scaleFactor(2.0f, 2.0f, 2.0f);

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), translationVector);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), scaleFactor);

    Camera camera;
    camera.pos = glm::vec3(0.0f, 1.0f, 15.0f);
    camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.up = glm::vec3(0.0f, 1.0f, 0.0f);

    float fov = 45.0f;
    //createProjectionMatrix(FOV, aspect ratio, nearClip, farClip)
    uniforms.projection = createProjectionMatrix(fov, FRAMEBUFFER_WIDTH / FRAMEBUFFER_HEIGHT, 0.01f, 1000.0f);
    uniforms.viewport = createViewportMatrix(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);

    glm::mat4 rotation = glm::mat4(1.0f);

    Planet jool, sun, laythe;
    std::vector<Planet> planets;

    jool.shader = joolShader;
    sun.shader = sunShader;
    laythe.shader = laytheShader;

    sun.model = createModelMatrix(translation, rotation, scale);
    sun.radius = 0;
    sun.rotationSpeed = 0.05f;

    jool.radius = 1;
    jool.rotationSpeed = 0.08f;
    jool.model = createModelMatrix(
        glm::translate(glm::mat4(1.0f), glm::vec3(jool.radius, 0.0f, jool.radius)),
        glm::rotate(glm::mat4(1.0f), jool.rotationSpeed, rotationAxis),
        glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f))
    );

    laythe.radius = 2;
    laythe.rotationSpeed = 0.03f;
    laythe.model = createModelMatrix(
        glm::translate(glm::mat4(1.0f), glm::vec3(jool.radius + laythe.radius, 0.0f, jool.radius + laythe.radius)),
        glm::rotate(glm::mat4(1.0f), laythe.rotationSpeed, rotationAxis),
        glm::scale(glm::mat4(1.0f), glm::vec3(0.6f, 0.6f, 0.6f))
    );

    planets.push_back(jool);
    planets.push_back(sun);
    planets.push_back(laythe);

    for( Planet& planet : planets ) {
        planet.vertexArray = vertexArray;
    }

    bool running = true;
    SDL_Event event;
    Uint32 frameStart, frameTime;

    initNoise();

    int osc = 0;
 
    L = camera.pos;

    while (running) {

        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                 switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        camera.pos.y += 1;
                        break;
                    case SDLK_LCTRL:
                        camera.pos.y -= 1;
                        break;
                    case SDLK_a:
                        camera.target.x -= cameraRadius * cos(angle);
                        camera.target.z -= cameraRadius * sin(angle);
                        break;
                    case SDLK_d:
                        camera.target.x += cameraRadius * cos(angle);
                        camera.target.z += cameraRadius * sin(angle);
                        break;
                    case SDLK_w:
                        camera.pos.z -= 1;
                        break;
                    case SDLK_s:
                        camera.pos.z += 1;
                        break;

                    case SDLK_g:
                        camera.pos = glm::vec3(0.0f, 20.0f, 1.0f);
                        camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
                        break;

                    case SDLK_f:
                        camera.pos = glm::vec3(0.0f, 1.0f, 15.0f);
                        break;
                }
            }
        }

        uniforms.view = createViewMatrix(camera);

        clear();

        #pragma omp parallel for
        for ( int i = 0; i < planets.size() ; i++ ) {
            planets[i].model = glm::rotate(planets[i].model, planets[i].rotationSpeed, rotationAxis);

            /*planets[i].model = glm::translate(
                planets[i].model,
                glm::vec3(
                    planets[i].radius * cos(planets[i].rotationSpeed),
                    0.0f,
                    planets[i].radius * sin(planets[i].rotationSpeed)
                )
            );*/

            renderPlanet(planets[i], uniforms);
        }

        renderBuffer(renderer);

        frameTime = SDL_GetTicks() - frameStart;

        if (frameTime > 0) {
            std::ostringstream titleStream;
            titleStream << "FPS: " << 1000.0 / frameTime << ", FOV: " << fov << ", L -> x" << L.x << " y" << L.y << " z" << L.z;
            SDL_SetWindowTitle(window, titleStream.str().c_str());
        }

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}