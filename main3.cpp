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

    if (abs(bary.z) < 1) {
        return std::make_pair(-1, -1);
    }

    return std::make_pair(
        bary.y / bary.z,
        bary.x / bary.z
    );    
}

void filledTriangle(const Vertex& a, const Vertex& b, const Vertex& c, std::vector<Fragment>& fragments, const glm::vec3 Light) {

  if( (a.clipSpace.z <= -0.5f && b.clipSpace.z <= -0.5f && c.clipSpace.z <= -0.5f) || 
  (a.clipSpace.w <= -0.5f && b.clipSpace.w <= -0.5f && c.clipSpace.w <= -0.5f) )
    return;

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

      double z2 = a.clipSpace.z * w + b.clipSpace.z * v + c.clipSpace.z * u;
      double w2 = a.clipSpace.w * w + b.clipSpace.w * v + c.clipSpace.w * u;

      if(z2 <= -5.0f && w2 <= -5.0f) continue;

      if (w < epsilon || v < epsilon || u < epsilon) continue;

      double z = A.z * w + B.z * v + C.z * u;

      if(z < epsilon) continue;

      float intensity = glm::dot( glm::normalize( a.normal * w + b.normal * v + c.normal * u), Light );

      glm::vec3 worldPos = a.worldPos * w + b.worldPos * v + c.worldPos * u;
      glm::vec3 originalPos = a.originalPos * w + b.originalPos * v + c.originalPos * u;

      fragments.push_back(
        Fragment{
          P,
          worldPos,
          originalPos,
          z,
          Color(255, 255, 255),
          intensity}
      );
    }
  }
}

void rasterizePlanet(const std::vector<std::vector<Vertex>>& assembledVertices, const Uniforms& uniforms, const glm::mat4& model, std::vector<Fragment>& fragments, const glm::vec3 Light) {

    for (std::vector<Vertex> triangleVertices : assembledVertices) {
        filledTriangle(triangleVertices[0], triangleVertices[1], triangleVertices[2], fragments, Light);
    }

}

void renderPlanet(Planet& planet, const Uniforms& uniforms) {

    std::vector<Vertex> transformedVertices(planet.vertexArray.size());

    #pragma omp parallel for
    for (int i = 0; i<planet.vertexArray.size(); i++){
        transformedVertices[i] = vertexShaderPlanet(planet.vertexArray[i], uniforms, planet.model);
    }
    
    std::vector<std::vector<Vertex>> assembledVertices = primitiveAssembly(transformedVertices);

    std::vector<Fragment> fragments;
    rasterizePlanet(assembledVertices, uniforms, planet.model, fragments, planet.calcLight(L));

    Fragment frag;
    
    for (int i = 0; i<fragments.size(); i++){
        frag = planet.shader(fragments[i]);
        point(frag);
    }

}

void renderMoon(Moon& Moon, const Uniforms& uniforms, const glm::vec3 parentTranslate) {

    std::vector<Vertex> transformedVertices(Moon.vertexArray.size());

    #pragma omp parallel for
    for (int i = 0; i<Moon.vertexArray.size(); i++){

        transformedVertices[i] = vertexShaderPlanet(Moon.vertexArray[i], uniforms, Moon.model);
    }
    
    std::vector<std::vector<Vertex>> assembledVertices = primitiveAssembly(transformedVertices);

    std::vector<Fragment> fragments;
    rasterizePlanet(assembledVertices, uniforms, Moon.model, fragments, Moon.calcLight(L, parentTranslate));

    Fragment frag;
    
    for (int i = 0; i<fragments.size(); i++){
        frag = Moon.shader(fragments[i]);
        point(frag);
    }

}

void renderRings(PlanetaryRing& rings, const Uniforms& uniforms) {

    std::vector<Vertex> transformedVertices(rings.vertexArray.size());

    #pragma omp parallel for
    for (int i = 0; i<rings.vertexArray.size(); i++){
        transformedVertices[i] = vertexShaderPlanet(rings.vertexArray[i], uniforms, rings.model);
    }
    
    std::vector<std::vector<Vertex>> assembledVertices = primitiveAssembly(transformedVertices);

    std::vector<Fragment> fragments;
    rasterizePlanet(assembledVertices, uniforms, rings.model, fragments, rings.calcLight(L));

    Fragment frag;
    
    for (int i = 0; i<fragments.size(); i++){
        frag = rings.shader(fragments[i]);
        point(frag);
    }

}

void renderShip(miscObj& ship, const Uniforms& uniforms) {

    std::vector<Vertex> transformedVertices(ship.vertexArray.size());

    #pragma omp parallel for
    for (int i = 0; i<ship.vertexArray.size(); i++){
        transformedVertices[i] = vertexShaderPlanet(ship.vertexArray[i], uniforms, ship.model);
    }
    
    std::vector<std::vector<Vertex>> assembledVertices = primitiveAssembly(transformedVertices);

    std::vector<Fragment> fragments;
    rasterizePlanet(assembledVertices, uniforms, ship.model, fragments, ship.calcLight(L));

    Fragment frag;
    
    for (int i = 0; i<fragments.size(); i++){
        frag = ship.shader(fragments[i]);
        point(frag);
    }

}

int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_EVERYTHING);

    std::vector<OrbitPoint> oPoints;

    init();
    initStarNoise();

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tex; //future-proofing
    std::vector<Face> faces;
    Uniforms uniforms;

    bool loaded = loadOBJ("sphere.obj", vertices, faces, normals, tex);
    std::vector<Vertex> vertexArray = setupVertexArray(vertices, faces, normals);

    glm::mat4 view = glm::mat4(1);
    glm::mat4 projection = glm::mat4(1);

    uniforms.model = glm::mat4(1.0f),
    uniforms.view = view;

    Camera camera;
    camera.pos = glm::vec3(0.0f, 0.0f, 15.0f);
    camera.target = glm::vec3(0.0f, 0.0f, 13.0f);
    camera.up = glm::vec3(0.0f, 1.0f, 0.0f);

    float fov = 45.0f;
    uniforms.projection = createProjectionMatrix(fov, FRAMEBUFFER_WIDTH / FRAMEBUFFER_HEIGHT, 1.0f, 1000.0f);
    uniforms.viewport = createViewportMatrix(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);

    Planet sun, rocky, gas, lean;
    Moon moon;
    PlanetaryRing rings;

    vertices = {};
    normals = {};
    tex = {};
    faces = {};

    loaded = loadOBJ("rings7.obj", vertices, faces, normals, tex);
    std::vector<Vertex> ringsVertexArray = setupVertexArray(vertices, faces, normals);
    rings.vertexArray = ringsVertexArray;

    std::vector<Planet> planets;

    sun.setOrbit(0, 2.0f, 1.5f);
    rocky.setOrbit(3.5, 1.3f, 2.0f);
    gas.setOrbit(9, 0.8f, 1.8f);
    lean.setOrbit(6.5, 1.0f, 1.7f);
    moon.setOrbit(1, 3.0f, 1.0f);
    sun.shader = sunShader;
    rocky.shader = joolShader;
    moon.shader = laytheShader;
    gas.shader = gasShader;
    lean.shader = marsShader;
    sun.scale = glm::vec3(2.0f, 2.0f, 2.0f);
    rocky.scale = glm::vec3(0.8f, 0.8f, 0.8f);
    moon.scale = glm::vec3(0.25f, 0.25f, 0.25f);
    gas.scale = glm::vec3(1.4f, 1.4f, 1.4f);
    lean.scale = glm::vec3(0.6f, 0.6f, 0.6f);
    sun.orbitColor = Color(0, 0, 0);
    rocky.orbitColor = Color(146, 180, 244);
    moon.orbitColor = Color(238, 193, 112);
    gas.orbitColor = Color(97, 231, 134);
    lean.orbitColor = Color(151, 146, 227);

    rings.scale = glm::vec3(0.6f, 0.6f, 0.6f);
    rings.shader = testShader;
    rings.setOrbit(gas.radius, gas.orbitSpeed);

    planets.push_back(sun);
    rocky.moons.push_back(moon);
    planets.push_back(rocky);
    planets.push_back(gas);
    planets.push_back(lean);
    
    for( Planet& planet : planets ) {
        planet.vertexArray = vertexArray;
        planet.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
        for( Moon& moon : planet.moons ) { 
            moon.vertexArray = vertexArray;
            moon.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
         }
    }

    bool running = true;
    SDL_Event event;
    Uint32 frameStart, frameTime;

    initNoise();

    //Load spaceship
    vertices = {};
    normals = {};
    tex = {};
    faces = {};

    miscObj ship;

    loaded = loadOBJ("nave2.obj", vertices, faces, normals, tex);
    std::vector<Vertex> shipVertexArray = setupVertexArray(vertices, faces, normals);
    ship.vertexArray = shipVertexArray;
    ship.scale = glm::vec3(0.07f, 0.07f, 0.07f);
    ship.shader = ringShader;
 
    int time = 0;
    float cameraAngle = M_PI;

    camera.target = glm::vec3(camera.pos.x + 2.0f * sin(cameraAngle), 0.0f, camera.pos.z + 2.0f * cos(cameraAngle));

    ship.setModel(cameraAngle, camera.target);

    uniforms.view = createViewMatrix(camera);

    while (running) {

        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                 switch (event.key.keysym.sym) {
                    case SDLK_a:
                        cameraAngle += 0.08f;
                        camera.target = glm::vec3(camera.pos.x + 2.0f * sin(cameraAngle), 0.0f, camera.pos.z + 2.0f * cos(cameraAngle));
                        uniforms.view = createViewMatrix(camera);
                        break;

                    case SDLK_d:
                        cameraAngle -= 0.08f;
                        camera.target = glm::vec3(camera.pos.x + 2.0f * sin(cameraAngle), 0.0f, camera.pos.z + 2.0f * cos(cameraAngle));
                        uniforms.view = createViewMatrix(camera);
                        break;

                    case SDLK_w:
                        camera.pos.z += 0.5f * cos(cameraAngle);
                        camera.pos.x += 0.5f * sin(cameraAngle);
                        camera.target.z += 0.5f * cos(cameraAngle);
                        camera.target.x += 0.5f * sin(cameraAngle);
                        uniforms.view = createViewMatrix(camera);
                        break;

                    case SDLK_s:
                        camera.pos.z -= 0.5f * cos(cameraAngle);
                        camera.pos.x -= 0.5f * sin(cameraAngle);
                        camera.target.z -= 0.5f * cos(cameraAngle);
                        camera.target.x -= 0.5f * sin(cameraAngle);
                        uniforms.view = createViewMatrix(camera);
                        break;

                    case SDLK_g:
                        camera.pos = glm::vec3(0.0f, 25.0f, 0.0f);
                        camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
                        camera.up = glm::vec3(0.0f, 0.0f, -1.0f);
                        uniforms.view = createViewMatrix(camera);
                        break;

                    case SDLK_f:
                        camera.pos = glm::vec3(0.0f, 0.0f, 15.0f);
                        camera.target = glm::vec3(0.0f, 0.0f, 13.0f);
                        camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
                        cameraAngle = M_PI;
                        uniforms.view = createViewMatrix(camera);
                        break;
                }
            }
        }

        clearFramebuffer(100, 10);

        std::vector<OrbitPoint> newPoints;

        for(int i = 0; i<oPoints.size(); i++) {
            if(oPoints[i].decayLight() > 0.0f) {
                glm::vec3 pointPos = glm::vec3(oPoints[i].getScreenSpace(uniforms));
                glm::vec4 clipSpace = uniforms.projection * uniforms.view * oPoints[i].pos;
                if( clipSpace.z <= -0.5f || clipSpace.w <= 0.5f ) continue;
                point( Fragment {
                        glm::ivec2(pointPos),
                        pointPos,
                        pointPos,
                        pointPos.z,
                        Color(oPoints[i].color.r * oPoints[i].intensity, oPoints[i].color.g * oPoints[i].intensity, oPoints[i].color.b * oPoints[i].intensity),
                        oPoints[i].intensity
                    }
                );
                newPoints.push_back(oPoints[i]);
            }
        }

        oPoints = newPoints;

        time++;
        #pragma omp parallel for
        for ( int i = 0; i < planets.size() ; i++ ) {
            planets[i].setModel(time);
            renderPlanet(planets[i], uniforms);

            oPoints.push_back(OrbitPoint{glm::vec4(planets[i].translation, 1.0f), planets[i].orbitColor});

            for( int j = 0; j < planets[i].moons.size(); j++ ) {
                planets[i].moons[j].setModel(time, planets[i].translation);
                renderMoon(planets[i].moons[j], uniforms, planets[i].translation);

                oPoints.push_back(OrbitPoint{glm::vec4(planets[i].moons[j].translation, 1.0f), planets[i].moons[j].orbitColor});
            }
        }

        ship.setModel(cameraAngle, camera.target);
        rings.setModel(time, planets[2].orbitSpeed);

        #pragma omp parallel 
        #pragma omp single 
        {
            #pragma omp task
            renderShip(ship, uniforms);

            #pragma omp task
            renderRings(rings, uniforms);

        }

        renderBuffer(renderer);

        frameTime = SDL_GetTicks() - frameStart;

        if (frameTime > 0) {
            std::ostringstream titleStream;
            titleStream << "FPS: " << 1000.0 / frameTime;
            SDL_SetWindowTitle(window, titleStream.str().c_str());
        }

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}